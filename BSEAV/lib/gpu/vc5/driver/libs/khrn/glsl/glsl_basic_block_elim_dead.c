/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/

#include "glsl_basic_block_elim_dead.h"

// Remove dead edges and determine live blocks
static void drop_dead(Map *live, BasicBlock *b)
{
   if (b && !glsl_map_get(live, b)) {
      glsl_map_put(live, b, b);
      if (!b->branch_cond || (b->branch_cond->flavour == DATAFLOW_CONST && b->branch_cond->u.constant.value == 0)) {
         // Branch never taken
         b->branch_cond = NULL;
         b->branch_target = NULL;
      }
      // TODO Could detect always-true branch conditions here and replace
      // fallthrough_target with branch_target in that case. However it is not
      // currently safe to do so as changing the fallthrough path can mean the
      // exit block is not always the last block in the reverse postorder,
      // which is assumed in various places throughout the compiler.
      drop_dead(live, b->branch_target);
      drop_dead(live, b->fallthrough_target);
   }
}

void glsl_basic_block_elim_dead(BasicBlock *entry)
{
   Map *all = glsl_basic_block_all_reachable(entry);
   Map *live = glsl_map_new();
   drop_dead(live, entry);
   GLSL_MAP_FOREACH(e, all) {
      if (!glsl_map_get(live, e->v))
         glsl_basic_block_delete(e->v);
   }
   glsl_map_delete(live);
   glsl_map_delete(all);
}
