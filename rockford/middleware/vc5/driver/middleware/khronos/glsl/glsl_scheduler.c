/*=============================================================================
Copyright (c) 2009 Broadcom Europe Limited.
All rights reserved.

Project  :  khronos
Module   :  Header file
File     :  $RCSfile: $
Revision :  $Revision: $

FILE DESCRIPTION
Takes a dataflow graph and schedules the nodes. The algorithm is designed to
help reduce register pressure.

We feed nodes directly to the allocator rather than producing a list which is
fed to the allocator in one go. This is because we may want to use feedback
from the allocator to decide which thing to schedule next.
=============================================================================*/
//#define OUTPUT_GRAPHVIZ_ON_SUCCESS
//#define OUTPUT_GRAPHVIZ_ON_FAILURE
#include "glsl_common.h"

#include "glsl_backflow.h"
#include "glsl_backflow_visitor.h"
#include "glsl_backend.h"
#include "glsl_backend_uniforms.h"

#include "helpers/v3d/v3d_tmu.h"
#include <stdlib.h>

#if defined(OUTPUT_GRAPHVIZ_ON_SUCCESS) || defined(OUTPUT_GRAPHVIZ_ON_FAILURE)
#define GRAPHVIZ
#include <stdio.h>
void glsl_print_backflow_from_roots(FILE *f, Backflow **roots, int num_roots, const BackflowChain *iodeps, int pass);

static int graphviz_file_num = 0;
static void graphviz(Backflow **roots, int num_roots, const BackflowChain *iodeps, int threads, bool success)
{
#ifndef OUTPUT_GRAPHVIZ_ON_SUCCESS
   if (success) return;
#endif
#ifndef OUTPUT_GRAPHVIZ_ON_FAILURE
   if (!success) return;
#endif
#ifndef NDEBUG
   char filename[20];
   FILE *f;
   /* The filename is of the form
      graph{shader number}-{success|fail}{threads}.txt */
   sprintf(filename,
           "graph%d-%s%d.txt",
           graphviz_file_num,
           success ? "S" : "F",
           threads);
   f = fopen(filename, "w");
   assert(f);
   glsl_print_backflow_from_roots(f, roots, num_roots, iodeps, BACKEND_PASS_PRINT);
   fclose(f);
#else
#endif
}
#endif

static void dpostv_init_backend_fields(Backflow *backend, void *data)
{
   BackflowPriorityQueue *age_queue = data;

   bool is_uniform = (backend->type == SIG && backend->sigbits == SIGBIT_LDUNIF);
   bool is_attribute = (backend->type == SIG && backend->sigbits == SIGBIT_LDVPM);

   backend->phase = 0;
   backend->reg = 0;
   backend->remaining_dependents = 0;
   for (int i = 0; i < BACKFLOW_DEP_COUNT; i++)
   {
      if (backend->dependencies[i] != NULL)
         backend->dependencies[i]->remaining_dependents++;
   }

   if (backend->type != SPECIAL_VOID && !is_uniform && !is_attribute)
   {
      backend->delay = -(int)backend->age;
      glsl_backflow_priority_queue_push(age_queue, backend);
   }
}

static bool all_tmu_deps_done(struct tmu_dep_s *dep) {
   while (dep != NULL) {
      if (!dep->l->done) return false;
      dep = dep->next;
   }
   return true;
}

struct sched_deps_s {
   BackflowChain consumers;
   BackflowChain suppliers;
};

struct thread_section {
   bool switch_valid;
   unsigned int input;
   unsigned int config;
   unsigned int output;
   int n_lookups;
   struct tmu_lookup_s *lookups[16];
};

static bool some_tmu_deps_in_section(struct tmu_dep_s *deps,
                                     const struct thread_section *section)
{
   while (deps != NULL) {
      for (int i=0; i<section->n_lookups; i++) {
         if (deps->l == section->lookups[i]) return true;
      }
      deps = deps->next;
   }
   return false;
}

static void iodep_texture(struct sched_deps_s *deps, Backflow *consumer, Backflow *supplier)
{
   if (consumer != NULL && supplier != NULL)
   {
      glsl_backflow_chain_append(&consumer->io_dependencies, supplier);
      glsl_backflow_chain_append(&deps->consumers, consumer);
      glsl_backflow_chain_append(&deps->suppliers, supplier);
   }
}

void order_texture_lookups(SchedBlock *block)
{
   struct tmu_lookup_s head = { .next = NULL };
   struct tmu_lookup_s *last_sorted = &head;

   if (block->tmu_lookups == NULL) return;

   while (block->tmu_lookups != NULL) {
      struct tmu_lookup_s *prev = NULL;

      struct tmu_lookup_s *best_lookup = NULL;
      struct tmu_lookup_s *best_prev = NULL;
      for (struct tmu_lookup_s *cur = block->tmu_lookups; cur; cur=cur->next) {
         assert(!cur->done);   /* Done ones are no longer in the list */

         if (all_tmu_deps_done(cur->tmu_deps)) {
            if (best_lookup == NULL || cur->age < best_lookup->age) {
               best_prev = prev;
               best_lookup = cur;
            }
         }
         prev = cur;
      }

      assert(best_lookup != NULL);
      if (best_prev == NULL) {
         assert(block->tmu_lookups == best_lookup);
         block->tmu_lookups = best_lookup->next;
      } else {
         assert(block->tmu_lookups != best_lookup);
         best_prev->next = best_lookup->next;
      }

      last_sorted->next = best_lookup;
      best_lookup->next = NULL;
      best_lookup->done = true;
      last_sorted = last_sorted->next;
   }
   block->tmu_lookups = head.next;
}

/* Add iodependencies between nodes to ensure correct TMU fifo operation
 * and to parallelise texture requests for performance.
 *
 * Returns the number of thread switches.  */
static uint32_t fix_texture_dependencies(SchedBlock *block,
                                         struct sched_deps_s *sched_deps,
                                         uint32_t thread_count)
{
   uint32_t input_fifo_size  = V3D_TMU_INPUT_FIFO_SIZE / thread_count;
   uint32_t config_fifo_size = V3D_TMU_INPUT_FIFO_SIZE / thread_count;
   uint32_t output_fifo_size = V3D_TMU_OUTPUT_FIFO_SIZE / thread_count;

   uint32_t section_count = 1;
   struct thread_section cur_section = { .switch_valid = false,
                                         .n_lookups = 0,
                                         .input  = 0,
                                         .config = 0,
                                         .output = 0 };

   /* sentinel is used to give NULL values for cases of no dependency */
   struct tmu_lookup_s sentinel = { .first_read = NULL,  .last_read = NULL,
                                    .first_write = NULL, .last_write = NULL };

   /* Divide the lookups into sections. Lookups are written in one section,
    * read in the next. Move on to the next section if the output fifo is full
    * or we need to read a previous lookup. Also move on (if possible) if the
    * input fifo is full. Moving on is only allowed if there is data in the
    * output_fifo, but this is always true in the first 2 conditions. */
   struct tmu_lookup_s *lookup;
   for (lookup = block->tmu_lookups; lookup != NULL; lookup=lookup->next) {
      if ( cur_section.switch_valid &&
           ( cur_section.input  + lookup->write_count  > input_fifo_size  ||
             cur_section.config + lookup->config_count > config_fifo_size ||
             cur_section.output + lookup->read_count   > output_fifo_size ||
             some_tmu_deps_in_section(lookup->tmu_deps, &cur_section)    ))
      {
         cur_section.switch_valid = false;
         cur_section.input  = 0;
         cur_section.config = 0;
         cur_section.output = 0;
         cur_section.n_lookups = 0;
         section_count++;
      }

      cur_section.input  += lookup->write_count;
      cur_section.config += lookup->config_count;
      cur_section.output += lookup->read_count;
      if (lookup->read_count > 0) {
         cur_section.switch_valid = true;
         cur_section.lookups[cur_section.n_lookups++] = lookup;
      }

      lookup->read_section = section_count;
   }

   if (cur_section.switch_valid) section_count++;

   /* It is possible for writes in this section to overfill the output fifo if
    * they are processed quickly. To avoid this reads may have to be done first.
    * Add "write_read" dependencies to enforce this. */

   uint32_t words_in_output_fifo = 0;
   for (lookup = block->tmu_lookups; lookup != NULL; lookup=lookup->next)
   {
      /* Calculate how much stuff has been put into the FIFOs after this texture lookup */
      words_in_output_fifo += lookup->read_count;
      lookup->words_in_output_fifo = words_in_output_fifo;
   }

   for (lookup = block->tmu_lookups; lookup != NULL; lookup=lookup->next) {
      assert(lookup->write_count <= input_fifo_size &&
             lookup->read_count  <= output_fifo_size   );

      /* Find the earliest texture lookup which, when read back, leaves enough
       * room in the output FIFO.  */
      if (output_fifo_size >= lookup->words_in_output_fifo)
         lookup->write_read_dependency = &sentinel;
      else {
         struct tmu_lookup_s *l = block->tmu_lookups;
         while (l != NULL)
         {
            if (lookup->words_in_output_fifo - l->words_in_output_fifo <= output_fifo_size)
               break;

            l = l->next;
         }
         assert(l != NULL && l != lookup);
         lookup->write_read_dependency = l;
      }
   }

   /* Create section delimiting nodes. thrsw if threaded, dummys otherwise */
   Backflow **switch_nodes = malloc(section_count*sizeof(Backflow *));
   switch_nodes[0] = NULL;

   for (unsigned i = 1; i < section_count; i++)
   {
      if (thread_count > 1)
         switch_nodes[i] = glsl_backflow_thrsw();
      else
         switch_nodes[i] = glsl_backflow_dummy();

      iodep_texture(sched_deps, switch_nodes[i], switch_nodes[i-1]);
   }

   /* Add dependencies: */
   {
      struct tmu_lookup_s *last = &sentinel;
      struct tmu_lookup_s *lookup = block->tmu_lookups;

      while (lookup != NULL)
      {
         /* 1. First write this time depends on previous triggering write */
         iodep_texture(sched_deps, lookup->first_write, last->last_write);

         /* 2. Read depends on the previous read */
         iodep_texture(sched_deps, lookup->first_read, last->last_read);

         /* 3. If writing would overflow the output fifo then it depends on the read which clears the fifo down to size */
         iodep_texture(sched_deps, lookup->first_write, lookup->write_read_dependency->last_read);

         /* 4. thrsw depends on the last write in the thread section */
         uint32_t read_section = lookup->read_section;
         if (read_section < section_count)
            iodep_texture(sched_deps, switch_nodes[read_section], lookup->last_write);

         /* 5. thrsw depends on the last read in the thread section */
         if (read_section + 1 < section_count)
            iodep_texture(sched_deps, switch_nodes[read_section+1], lookup->last_read);

         /* 6. If read is the first in a thread section then it depends on the thrsw */
         if (read_section < section_count)
            iodep_texture(sched_deps, lookup->first_read, switch_nodes[read_section]);

         /* 7. The first write in a thread section depends on the previous thrsw */
         if (read_section > 0)
            iodep_texture(sched_deps, lookup->first_write, switch_nodes[read_section-1]);

         last = lookup;
         lookup = lookup->next;
      }
   }

   /* Force any TLB writes after the last read */
   iodep_texture(sched_deps, block->first_tlb_write, block->last_tlb_read);

   if (thread_count > 1 && section_count > 1) {
      /* Force any TLB reads after the last threadswitch */
      iodep_texture(sched_deps, block->first_tlb_read, switch_nodes[section_count-1]);

      /* Force any TLB writes after the last threadswitch */
      iodep_texture(sched_deps, block->first_tlb_write, switch_nodes[section_count-1]);

      /* Force all vpm reads/writes into the first/last thread section */
      if (block->last_vpm_read != NULL)
         iodep_texture(sched_deps, switch_nodes[1], block->last_vpm_read);
      if (block->first_vpm_write != NULL)
         iodep_texture(sched_deps, block->first_vpm_write, switch_nodes[section_count-1]);
   }

   free(switch_nodes);

   return (thread_count > 1) ? section_count-1 : 0;
}

static void clean_up_scheduler_iodeps(BackflowChain *consumers, BackflowChain *suppliers)
{
   BackflowChainNode *cnode, *snode;
   Backflow *consumer, *supplier;

   assert(consumers->count == suppliers->count);
   cnode = consumers->head;
   snode = suppliers->head;
   while (cnode != NULL)
   {
      assert(snode != NULL);
      consumer = cnode->ptr;
      supplier = snode->ptr;

      glsl_backflow_chain_remove(&consumer->io_dependencies, supplier);

      cnode = cnode->l.next;
      snode = snode->l.next;
   }
   assert(snode == NULL);
}

int get_max_threadability(struct tmu_lookup_s *tmu_lookups)
{
   int max = 4;
   bool threaded = false;
   /* If a shader makes no texture lookups it can't run threaded */
   for ( ; tmu_lookups != NULL; tmu_lookups = tmu_lookups->next) {
      /* If no lookups ever read (ie. writes) then we can't thread switch */
      if (tmu_lookups->read_count > 0) threaded = true;
      /* 4-threaded the FIFO has size 4, so if any request is larger we must
       * run 2-threaded so that it fits.                                    */
      if (tmu_lookups->write_count > 4) max = 2;
   }

   return threaded ? max : 1;
}

GENERATED_SHADER_T *glsl_backend_schedule(SchedBlock *block,
                                          RegList *presched,
                                          RegList *outputs,
                                          int blackout,
                                          glsl_binary_shader_flavour_t type,
                                          int threads,
                                          bool last,
                                          bool lthrsw)
{
   GENERATED_SHADER_T *result;

   BackflowPriorityQueue age_queue;
   glsl_backflow_priority_queue_init(&age_queue, 0x40000);

   struct sched_deps_s sched_deps;
   glsl_backflow_chain_init(&sched_deps.consumers);
   glsl_backflow_chain_init(&sched_deps.suppliers);

   uint32_t thrsw_count = fix_texture_dependencies(block, &sched_deps, threads);

   /* TODO: This is messy, make it simpler and more consistent */
   for (int i=0; i<block->num_outputs; i++)
      glsl_backflow_accept_towards_leaves_postfix(block->outputs[i], &age_queue,
                                                  dpostv_init_backend_fields,
                                                  BACKEND_PASS_SCHEDULE + 4*threads);

   for (BackflowChainNode *n=block->iodeps.head; n; n=n->l.next)
      glsl_backflow_accept_towards_leaves_postfix(n->ptr, &age_queue,
                                                  dpostv_init_backend_fields,
                                                  BACKEND_PASS_SCHEDULE + 4*threads);

   for (RegList *o = outputs; o; o=o->next) o->node->remaining_dependents++;

   /* XXX The output list is backwards. Damn. */
   int branch_cond = block->branch_cond == -1 ? -1 : block->num_outputs - 1 - block->branch_cond;
   result = glsl_backend(&block->iodeps, type, thrsw_count, threads, &age_queue,
                         presched, outputs, branch_cond, blackout, last, lthrsw);

#ifdef GRAPHVIZ
   graphviz_file_num++;
   graphviz(block->outputs, block->num_outputs, &block->iodeps, threads, (result != NULL));
#endif

   /* Clean up the texture iodependencies so we can try again with different threadability */
   clean_up_scheduler_iodeps(&sched_deps.consumers, &sched_deps.suppliers);

   return result;
}
