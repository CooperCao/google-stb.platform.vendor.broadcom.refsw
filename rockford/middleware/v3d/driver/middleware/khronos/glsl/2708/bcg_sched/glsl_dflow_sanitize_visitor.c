/*=============================================================================
Copyright (c) 2013 Broadcom Europe Limited.
All rights reserved.

Project  :  khronos
Module   :  BCG's scheduler

FILE DESCRIPTION

=============================================================================*/

#include "middleware/khronos/glsl/glsl_common.h"
#include "middleware/khronos/glsl/2708/bcg_sched/glsl_dflow_sanitize_visitor.h"
#include "middleware/khronos/glsl/2708/bcg_sched/glsl_dflow.h"
#include <stdio.h>

static void Destroy(void *me)
{
   DFlowSanitizeVisitor *self = (DFlowSanitizeVisitor *)me;

   NodeVector_Destr(&self->m_removeList);
}

static void Pass1(DFlowSanitizeVisitor *self, DFlowNode *node)
{
   // Some trees seem to have some zombie parents.
   // Let's remove those

   // Check if our parent lists us as a child
   NodeList_const_iterator piter;
   const NodeList *parents = DFlowNode_Parents(node);
   const NodeList *ioParents = DFlowNode_IoParents(node);
   uint32_t       i;

   for (piter = NodeList_const_begin(parents); piter != NodeList_const_end(parents); NodeList_const_next(&piter))
   {
      DFlowNode   *starPiter = NodeList_const_star(piter);

      const NodeList *pkids = DFlowNode_Children(starPiter);
      const NodeList *piokids = DFlowNode_IoChildren(starPiter);

      if (!DFlowNode_ListContains(pkids, node) && !DFlowNode_ListContains(piokids, node))
      {
         // Our parents can't see us!
         NodeVector_push_back(&self->m_removeList, starPiter);
         self->m_stuffRemoved = true;
      }
   }

   for (i = 0; i < NodeVector_size(&self->m_removeList); i++)
      NodeList_remove(&node->m_parents, NodeVector_index(&self->m_removeList, i));

   NodeVector_clear(&self->m_removeList);

   for (piter = NodeList_const_begin(ioParents); piter != NodeList_const_end(ioParents); NodeList_const_next(&piter))
   {
      DFlowNode   *starPiter = NodeList_const_star(piter);

      const NodeList *pkids = DFlowNode_Children(starPiter);
      const NodeList *piokids = DFlowNode_IoChildren(starPiter);

      if (!DFlowNode_ListContains(pkids, node) && !DFlowNode_ListContains(piokids, node))
      {
         // Our parents can't see us!
         NodeVector_push_back(&self->m_removeList, starPiter);
         self->m_stuffRemoved = true;
      }
   }

   for (i = 0; i < NodeVector_size(&self->m_removeList); i++)
      NodeList_remove(&node->m_ioParents, NodeVector_index(&self->m_removeList, i));

   NodeVector_clear(&self->m_removeList);

   // Use the opaque data to store the fact that we visited this node top-down
   // TODO : these really need to be cleared at the end (add another pass)
   node->m_visitorOpaqueData = (void*)self;
}

static void Pass2(DFlowSanitizeVisitor *self, DFlowNode *node)
{
   // Remove any linked parent nodes that don't have the visited flag set in the opaque data
   NodeList_const_iterator piter;
   const NodeList *parents = DFlowNode_Parents(node);
   const NodeList *ioParents = DFlowNode_IoParents(node);
   uint32_t       i;

   for (piter = NodeList_const_begin(parents); piter != NodeList_const_end(parents); NodeList_const_next(&piter))
   {
      DFlowNode   *starPiter = NodeList_const_star(piter);

      if (starPiter->m_visitorOpaqueData != (void*)self)
      {
         NodeVector_push_back(&self->m_removeList, starPiter);
         self->m_stuffRemoved = true;
      }
   }

   for (i = 0; i < NodeVector_size(&self->m_removeList); i++)
      NodeList_remove(&node->m_parents, NodeVector_index(&self->m_removeList, i));

   NodeVector_clear(&self->m_removeList);

   for (piter = NodeList_const_begin(ioParents); piter != NodeList_const_end(ioParents); NodeList_const_next(&piter))
   {
      DFlowNode   *starPiter = NodeList_const_star(piter);

      if (starPiter->m_visitorOpaqueData != (void*)self)
      {
         NodeVector_push_back(&self->m_removeList, starPiter);
         self->m_stuffRemoved = true;
      }
   }

   for (i = 0; i < NodeVector_size(&self->m_removeList); i++)
      NodeList_remove(&node->m_ioParents, NodeVector_index(&self->m_removeList, i));
}

static void Accept(void *me, DFlowNode *node)
{
   DFlowSanitizeVisitor *self = (DFlowSanitizeVisitor *)me;

   if (self->m_pass == 1)
      Pass1(self, node);
   else
      Pass2(self, node);
}

void DFlowSanitizeVisitor_Constr(DFlowSanitizeVisitor *self, DFlowRecursionOptimizer *opt)
{
   DFlowVisitor_Constr(self, Destroy, Accept, opt);

   self->m_stuffRemoved = false;
   self->m_pass         = 0;

   NodeVector_Constr(&self->m_removeList, 16);
}

void DFlowSanitizeVisitor_Visit(DFlowSanitizeVisitor *self, DFlowNode *node)
{
   self->m_pass = 1;

   DFlowVisitor_VisitTopDown(self, node);

   if (self->m_stuffRemoved)
      DFlowVisitor_InvalidateRecursionOptimizer(self); // We've changed the graph, we must invalidate the recursion optimizer

   self->m_stuffRemoved = false;
   self->m_pass = 2;

   DFlowVisitor_VisitTopDown(self, node);

   if (self->m_stuffRemoved)
      DFlowVisitor_InvalidateRecursionOptimizer(self); // We've changed the graph, we must invalidate the recursion optimizer
}
