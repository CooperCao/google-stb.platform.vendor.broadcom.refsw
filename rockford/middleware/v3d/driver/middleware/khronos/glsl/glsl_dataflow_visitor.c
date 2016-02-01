/*=============================================================================
Copyright (c) 2008 Broadcom Europe Limited.
All rights reserved.

Project  :  khronos
Module   :

FILE DESCRIPTION
Standalone GLSL compiler
=============================================================================*/
#include "middleware/khronos/glsl/glsl_common.h"

#include <stdlib.h>

#include "middleware/khronos/glsl/glsl_dataflow_visitor.h"
#include "middleware/khronos/glsl/glsl_stack.h"

void glsl_dataflow_accept_towards_leaves(Dataflow* dataflow, void* data, DataflowPreVisitor dprev, DataflowPostVisitor dpostv, int pass)
{
   STACK_CHECK();

   if (dataflow && dataflow->pass != pass)
   {
      DataflowChainNode *node;
      int i;

      dataflow->pass = pass;

      if (dprev)
      {
         Dataflow* alt_dataflow = dprev(dataflow, data);

         if (alt_dataflow != dataflow)
         {
            glsl_dataflow_accept_towards_leaves(alt_dataflow, data, dprev, dpostv, pass);
            return;
         }
      }

      for (i=0; i<dataflow->dependencies_count; ++i) {
         glsl_dataflow_accept_towards_leaves(dataflow->d.dependencies[i],
            data, dprev, dpostv, pass);
      }

      for (node = dataflow->iodependencies.first; node; node = node->next)
         glsl_dataflow_accept_towards_leaves(node->dataflow, data, dprev, dpostv, pass);

      if (dpostv) dpostv(dataflow, data);
   }
}

void glsl_dataflow_accept_towards_leaves_prefix(Dataflow* dataflow, void* data, DataflowPreVisitor dprev, int pass)
{
   glsl_dataflow_accept_towards_leaves(dataflow, data, dprev, NULL, pass);
}

void glsl_dataflow_accept_towards_leaves_postfix(Dataflow* dataflow, void* data, DataflowPostVisitor dpostv, int pass)
{
   glsl_dataflow_accept_towards_leaves(dataflow, data, NULL, dpostv, pass);
}


void glsl_dataflow_accept_from_leaves(DataflowChain* pool, void* data, DataflowPreVisitor dprev, DataflowPostVisitor dpostv, int pass)
{
   DataflowChainNode* node;

   STACK_CHECK();

   for (node = pool->first; node; node = node->next)
   {
      Dataflow* dataflow = node->dataflow;

      if (dataflow && dataflow->pass != pass)
      {
         dataflow->pass = pass;

         if (dprev) dataflow = dprev(dataflow, data);
         if (!dataflow) return;

         glsl_dataflow_accept_from_leaves(&dataflow->dependents, data, dprev, dpostv, pass);
         glsl_dataflow_accept_from_leaves(&dataflow->iodependents, data, dprev, dpostv, pass);

         if (dpostv) dpostv(dataflow, data);
      }
   }
}

void glsl_dataflow_accept_from_leaves_prefix(DataflowChain* pool, void* data, DataflowPreVisitor dprev, int pass)
{
   glsl_dataflow_accept_from_leaves(pool, data, dprev, NULL, pass);
}

void glsl_dataflow_accept_from_leaves_postfix(DataflowChain* pool, void* data, DataflowPostVisitor dpostv, int pass)
{
   glsl_dataflow_accept_from_leaves(pool, data, NULL, dpostv, pass);
}
