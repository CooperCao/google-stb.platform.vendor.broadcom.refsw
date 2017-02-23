/*=============================================================================
Broadcom Proprietary and Confidential. (c)2013 Broadcom.
All rights reserved.

Project  :  khronos
Module   :  BCG's scheduler

FILE DESCRIPTION

=============================================================================*/

#include "middleware/khronos/glsl/2708/bcg_sched/glsl_dflow_visitor.h"
#include "middleware/khronos/glsl/2708/bcg_sched/glsl_dflow.h"

///////////////////////////////////////////////////////////////////////////////
// DFlowRecursionOptimizer
///////////////////////////////////////////////////////////////////////////////

void DFlowRecursionOptimizer_Constr(DFlowRecursionOptimizer *self)
{
   NodeVector_Constr(&self->m_topDown,      128);
   NodeVector_Constr(&self->m_bottomUp,     128);
   NodeVector_Constr(&self->m_breadthFirst, 128);

   self->m_root = NULL;
}

void DFlowRecursionOptimizer_Destr(DFlowRecursionOptimizer *self)
{
   NodeVector_Destr(&self->m_topDown);
   NodeVector_Destr(&self->m_bottomUp);
   NodeVector_Destr(&self->m_breadthFirst);
}

void DFlowRecursionOptimizer_Invalidate(DFlowRecursionOptimizer *self)
{
   NodeVector_clear(&self->m_topDown);
   NodeVector_clear(&self->m_bottomUp);
   NodeVector_clear(&self->m_breadthFirst);
}

///////////////////////////////////////////////////////////////////////////////
// DFlowVisitor
///////////////////////////////////////////////////////////////////////////////

// Private
static void DFlowVisitor_RecurseBottomUp(void *me, DFlowNode *root)
{
   DFlowVisitor  *self = (DFlowVisitor *)me;

   NodeList_const_iterator iter;

   const NodeList *kids = DFlowNode_Children(root);
   const NodeList *ioKids = DFlowNode_IoChildren(root);

   if (root->m_visitorId != DFlowNode_lastVisitorId)
   {
      root->m_visitorId = DFlowNode_lastVisitorId; // Prevent re-visit

      for (iter = NodeList_const_begin(kids); iter != NodeList_const_end(kids); NodeList_const_next(&iter))
         DFlowVisitor_RecurseBottomUp(self, NodeList_const_star(iter));

      for (iter = NodeList_const_begin(ioKids); iter != NodeList_const_end(ioKids); NodeList_const_next(&iter))
         DFlowVisitor_RecurseBottomUp(self, NodeList_const_star(iter));

      NodeVector_push_back(&self->m_opt->m_bottomUp, root);
   }
}

static void DFlowVisitor_RecurseTopDown(void *me, DFlowNode *root)
{
   DFlowVisitor  *self = (DFlowVisitor *)me;

   NodeList_const_iterator iter;

   const NodeList *kids = DFlowNode_Children(root);
   const NodeList *ioKids = DFlowNode_IoChildren(root);

   if (root->m_visitorId != DFlowNode_lastVisitorId)
   {
      root->m_visitorId = DFlowNode_lastVisitorId; // Prevent re-visit

      NodeVector_push_back(&self->m_opt->m_topDown, root);

      for (iter = NodeList_const_begin(kids); iter != NodeList_const_end(kids); NodeList_const_next(&iter))
         DFlowVisitor_RecurseTopDown(self, NodeList_const_star(iter));

      for (iter = NodeList_const_begin(ioKids); iter != NodeList_const_end(ioKids); NodeList_const_next(&iter))
         DFlowVisitor_RecurseTopDown(self, NodeList_const_star(iter));
   }
}

void DFlowVisitor_VisitBottomUp(void *me, DFlowNode *root)
{
   DFlowVisitor  *self = (DFlowVisitor *)me;

   if (root != DFlowRecursionOptimizer_Root(self->m_opt))
   {
      DFlowRecursionOptimizer_Invalidate(self->m_opt);
      DFlowRecursionOptimizer_SetRoot(self->m_opt, root);
   }

   if (NodeVector_size(&self->m_opt->m_bottomUp) == 0)
   {
      // Build optimized order list.
      // Initial entry point for visiting. Reset id here.
      DFlowNode_lastVisitorId++;
      DFlowVisitor_RecurseBottomUp(self, root);
   }

   {
      uint32_t len = NodeVector_size(&self->m_opt->m_bottomUp);
      uint32_t i;

      for (i = 0; i < len; i++)
         DFlowVisitor_Accept(self, NodeVector_index(&self->m_opt->m_bottomUp, i));
   }
}

void DFlowVisitor_VisitTopDown(void *me, DFlowNode *root)
{
   DFlowVisitor  *self = (DFlowVisitor *)me;

   if (root != DFlowRecursionOptimizer_Root(self->m_opt))
   {
      DFlowRecursionOptimizer_Invalidate(self->m_opt);
      DFlowRecursionOptimizer_SetRoot(self->m_opt, root);
   }

   if (NodeVector_size(&self->m_opt->m_topDown) == 0)
   {
      // Build optimized order list.
      // Initial entry point for visiting. Reset id here.
      DFlowNode_lastVisitorId++;
      DFlowVisitor_RecurseTopDown(self, root);
   }

   {
      uint32_t len = NodeVector_size(&self->m_opt->m_topDown);
      uint32_t i;

      for (i = 0; i < len; i++)
         DFlowVisitor_Accept(self, NodeVector_index(&self->m_opt->m_topDown, i));
   }
}

void DFlowVisitor_VisitBreadthFirst(void *me, DFlowNode *root)
{
   DFlowVisitor  *self = (DFlowVisitor *)me;

   if (root != DFlowRecursionOptimizer_Root(self->m_opt))
   {
      DFlowRecursionOptimizer_Invalidate(self->m_opt);
      DFlowRecursionOptimizer_SetRoot(self->m_opt, root);
   }

   if (NodeVector_size(&self->m_opt->m_breadthFirst) == 0)
   {
      // Initial entry point for visiting. Reset id here.
      NodeList_const_iterator iter;
      NodeList q;

      DFlowNode_lastVisitorId++;

      NodeList_Constr(&q);

      NodeList_push_back(&q, root);

      while (NodeList_size(&q) != 0)
      {
         DFlowNode      *node = NodeList_front(&q);
         const NodeList *parents;
         const NodeList *ioParents;
         bool           allParentsVisited = true;

         NodeList_pop(&q);

         // Check if all our parents have been visited
         parents = DFlowNode_Parents(node);
         ioParents = DFlowNode_IoParents(node);

         for (iter = NodeList_const_begin(parents); iter != NodeList_const_end(parents); NodeList_const_next(&iter))
         {
            if (NodeList_const_star(iter)->m_visitorId != DFlowNode_lastVisitorId)
            {
               allParentsVisited = false;
               break;
            }
         }

         for (iter = NodeList_const_begin(ioParents); iter != NodeList_const_end(ioParents); NodeList_const_next(&iter))
         {
            if (NodeList_const_star(iter)->m_visitorId != DFlowNode_lastVisitorId)
            {
               allParentsVisited = false;
               break;
            }
         }

         if (allParentsVisited && node->m_visitorId != DFlowNode_lastVisitorId)
         {
            const NodeList *kids;
            const NodeList *ioKids;

            NodeVector_push_back(&self->m_opt->m_breadthFirst, node);
            //Accept(node);
            node->m_visitorId = DFlowNode_lastVisitorId;

            kids = DFlowNode_Children(node);
            ioKids = DFlowNode_IoChildren(node);

            for (iter = NodeList_const_begin(kids); iter != NodeList_const_end(kids); NodeList_const_next(&iter))
               NodeList_push_back(&q, NodeList_const_star(iter));

            for (iter = NodeList_const_begin(ioKids); iter != NodeList_const_end(ioKids); NodeList_const_next(&iter))
               NodeList_push_back(&q, NodeList_const_star(iter));
         }
      }
   }

   {
      uint32_t len = NodeVector_size(&self->m_opt->m_breadthFirst);
      uint32_t i;

      for (i = 0; i < len; i++)
         DFlowVisitor_Accept(self, NodeVector_index(&self->m_opt->m_breadthFirst, i));
   }
}
