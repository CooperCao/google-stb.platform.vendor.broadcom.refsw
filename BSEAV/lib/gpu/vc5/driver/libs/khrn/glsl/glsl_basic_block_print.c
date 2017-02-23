/*=============================================================================
Broadcom Proprietary and Confidential. (c)2015 Broadcom.
All rights reserved.

Project  :  glsl
Module   :

FILE DESCRIPTION
=============================================================================*/

#include "glsl_basic_block_print.h"

void glsl_basic_block_print(FILE *f, BasicBlock *entry)
{
   BasicBlockList *blocks = glsl_basic_block_get_reverse_postorder_list(entry);

   int id = 0;
   for (BasicBlockList *node = blocks; node; node=node->next) {
      fprintf(f, "\tn%p [label=%d]\n", node->v, id++);
   }

   for (BasicBlockList *node = blocks; node; node=node->next) {
      if (node->v->fallthrough_target)
         fprintf(f, "\tn%p -> \tn%p\n", node->v, node->v->fallthrough_target);
      if (node->v->branch_target)
         fprintf(f, "\tn%p -> \tn%p\n", node->v, node->v->branch_target);
   }
   fflush(f);
}
