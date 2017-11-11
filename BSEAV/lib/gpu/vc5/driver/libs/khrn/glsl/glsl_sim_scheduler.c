/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "glsl_sim_scheduler.h"
#include "glsl_map.h"

typedef enum sched_state
{
   STATE_UNVISITED,
#ifndef NDEBUG
   STATE_STACKED,
#endif
   STATE_COMPLETE
} sched_state;

typedef struct sim_scheduler
{
   Backflow** stack;
   unsigned stack_capacity;
   unsigned stack_size;
   Map* state_map;
   glsl_sim_schedule_node schedule_node;
   void* schedule_context;
} sim_scheduler;

static inline sched_state get_state(sim_scheduler* sched, Backflow* node)
{
   return (sched_state)(uintptr_t)glsl_map_get(sched->state_map, node);
}

static inline void set_state(sim_scheduler* sched, Backflow* node, sched_state state)
{
   glsl_map_put(sched->state_map, node, (void*)(uintptr_t)state);
}

static void stack_push(sim_scheduler* sched, Backflow* node)
{
   assert(get_state(sched, node) == STATE_UNVISITED);
   if (sched->stack_size == sched->stack_capacity)
   {
      sched->stack_capacity = sched->stack_capacity ? sched->stack_capacity * 2 : 1024;
      sched->stack = glsl_safemem_realloc(sched->stack, sched->stack_capacity * sizeof(Backflow *));
      assert(sched->stack_capacity > sched->stack_size);
   }

   debug_only(set_state(sched, node, STATE_STACKED));
   sched->stack[sched->stack_size++] = node;
}

static Backflow *get_unvisited_dep(sim_scheduler* sched, Backflow *node)
{
   for (BackflowIODepChainNode *n=node->io_dependencies.head; n; n=n->next)
   {
      unsigned state = get_state(sched, n->val.dep);
      assert(state != STATE_STACKED);
      if (state == STATE_UNVISITED)
         return n->val.dep;
   }

#if V3D_VER_AT_LEAST(4,1,34,0)
   v3d_qpu_sigbits_t defer_unifs = V3D_QPU_SIG_LDUNIF | V3D_QPU_SIG_LDUNIFRF | V3D_QPU_SIG_LDUNIFA | V3D_QPU_SIG_LDUNIFARF;
#else
   v3d_qpu_sigbits_t defer_unifs = V3D_QPU_SIG_LDUNIF;
#endif
   for (unsigned pass = 0; pass != 2; ++pass) {
      for (unsigned i = 0; i < BACKFLOW_DEP_COUNT; i++) {
         Backflow* dep = node->dependencies[i];
         if (dep == NULL) continue;
         if (dep->type == SIG && (dep->u.sigbits & defer_unifs)) continue;

         unsigned state = get_state(sched, dep);
         assert(state != STATE_STACKED);
         if (state == STATE_UNVISITED)
            return dep;
      }
      defer_unifs = 0;
   }
   return NULL;
}

static void visit_recursive(sim_scheduler* sched, Backflow* root)
{
   if (get_state(sched, root) >= STATE_COMPLETE)
      return;

   stack_push(sched, root);

   while (sched->stack_size != 0)
   {
      Backflow *node = sched->stack[sched->stack_size-1];
      Backflow *dep = get_unvisited_dep(sched, node);
      if (dep != NULL)
      {
         stack_push(sched, dep);
      }
      else
      {
         assert(get_state(sched, node) == STATE_STACKED);
         set_state(sched, node, STATE_COMPLETE);
         sched->stack_size -= 1;
         sched->schedule_node(sched->schedule_context, node);
      }
   }
}

void glsl_sim_schedule(glsl_sim_schedule_params* p)
{
   sim_scheduler sched = {
      .state_map = glsl_map_new(),
      .schedule_node = p->schedule_node,
      .schedule_context = p->schedule_context
   };

   for (int i = 0; i != p->count; ++i)
      visit_recursive(&sched, p->queue[i]);

   for (RegList *r = p->outputs; r; r = r->next)
      visit_recursive(&sched, r->node);

   for (BackflowChainNode *n = p->iodeps->head; n; n = n->next)
      visit_recursive(&sched, n->val);

   if (p->branch_cond)
      visit_recursive(&sched, p->branch_cond);

   glsl_safemem_free(sched.stack);
   glsl_map_delete(sched.state_map);
}
