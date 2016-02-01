/*=============================================================================
Copyright (c) 2014 Broadcom Europe Limited.
All rights reserved.

Project  :  glsl
Module   :

FILE DESCRIPTION
=============================================================================*/

#include <stdlib.h>

#include "glsl_common.h"
#include "glsl_backflow.h"
#include "glsl_backflow_visitor.h"

void glsl_backflow_accept_towards_leaves(Backflow* backflow, void* data, BackflowPreVisitor dprev, BackflowPostVisitor dpostv, int pass)
{
   if (backflow && backflow->pass != pass)
   {
      BackflowChainNode *node;
      uint32_t i;

      backflow->pass = pass;

      if (dprev)
      {
         Backflow* alt_backflow = dprev(backflow, data);

         if (alt_backflow != backflow)
         {
            glsl_backflow_accept_towards_leaves(alt_backflow, data, dprev, dpostv, pass);
            return;
         }
      }

      for (i=0; i<BACKFLOW_DEP_COUNT; ++i) {
         glsl_backflow_accept_towards_leaves(backflow->dependencies[i],
                                             data, dprev, dpostv, pass);
      }

      LIST_FOR_EACH(node, &backflow->io_dependencies, l)
         glsl_backflow_accept_towards_leaves(node->ptr, data, dprev, dpostv, pass);

      if (dpostv) dpostv(backflow, data);
   }
}

void glsl_backflow_accept_towards_leaves_prefix(Backflow* backflow, void* data, BackflowPreVisitor dprev, int pass)
{
   glsl_backflow_accept_towards_leaves(backflow, data, dprev, NULL, pass);
}

void glsl_backflow_accept_towards_leaves_postfix(Backflow* backflow, void* data, BackflowPostVisitor dpostv, int pass)
{
   glsl_backflow_accept_towards_leaves(backflow, data, NULL, dpostv, pass);
}
