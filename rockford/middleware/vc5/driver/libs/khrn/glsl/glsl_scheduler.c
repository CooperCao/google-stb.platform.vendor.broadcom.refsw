/*=============================================================================
Broadcom Proprietary and Confidential. (c)2009 Broadcom.
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

#include "glsl_backflow.h"
#include "glsl_backflow_visitor.h"
#include "glsl_backend.h"
#include "glsl_backend_uniforms.h"

#include "glsl_sched_node_helpers.h"

#include "libs/core/v3d/v3d_tmu.h"
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
   /* The filename is of the form
      graph{shader number}-{success|fail}{threads}.txt */
   sprintf(filename,
           "graph%d-%s%d.txt",
           graphviz_file_num,
           success ? "S" : "F",
           threads);
   FILE *f = fopen(filename, "w");
   glsl_print_backflow_from_roots(f, roots, num_roots, iodeps, BACKEND_PASS_PRINT);
   fclose(f);
#endif
}
#endif

static void dpostv_init_backend_fields(Backflow *backend, void *data)
{
   BackflowPriorityQueue *age_queue = data;

   bool is_uniform = (backend->type == SIG && backend->u.sigbits == SIGBIT_LDUNIF);
#if !V3D_HAS_LDVPM
   bool is_attribute = (backend->type == SIG && backend->u.sigbits == SIGBIT_LDVPM);
#else
   bool is_attribute = false;
#endif

   backend->phase = 0;
   backend->reg = 0;
   backend->remaining_dependents = 0;
   for (int i = 0; i < BACKFLOW_DEP_COUNT; i++)
   {
      if (backend->dependencies[i] != NULL)
         backend->dependencies[i]->remaining_dependents++;
   }

   if (backend->type != SPECIAL_VOID && !is_uniform && !is_attribute)
      glsl_backflow_priority_queue_push(age_queue, backend);
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
   Backflow *pre_write;
   Backflow *pre_read;

   unsigned int input;
   unsigned int config;
   unsigned int output;

   int n_read_lookups;
   struct tmu_lookup_s *read_lookups[V3D_TMU_INPUT_FIFO_SIZE];
};

static bool some_tmu_deps_in_section(struct tmu_dep_s *deps,
                                     const struct thread_section *section)
{
   while (deps != NULL) {
      for (int i=0; i<section->n_read_lookups; i++) {
         if (deps->l == section->read_lookups[i]) return true;
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

         if (all_tmu_deps_done(cur->last_write->tmu_deps)) {
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

static void start_new_section(struct thread_section *sec, Backflow *pre_write, Backflow *pre_read) {
   sec->pre_write = pre_write;
   sec->pre_read  = pre_read;

   sec->input  = 0;
   sec->config = 0;
   sec->output = 0;
   sec->n_read_lookups = 0;
}

/* Add iodependencies between nodes to ensure correct TMU fifo operation
 * and to parallelise texture requests for performance.
 *
 * Returns the number of thread switches.  */
static uint32_t fix_texture_dependencies(SchedBlock *block,
                                         struct sched_deps_s *sched_deps,
                                         uint32_t thread_count,
                                         bool sbwaited)
{
   uint32_t input_fifo_size  = V3D_TMU_INPUT_FIFO_SIZE / thread_count;
   uint32_t config_fifo_size = V3D_TMU_CONFIG_FIFO_SIZE / thread_count;
   uint32_t output_fifo_size = V3D_TMU_OUTPUT_FIFO_SIZE / thread_count;

   /* Divide the lookups into sections. Lookups are written in one section,
    * read in the next. Move on to the next section if the output fifo is full
    * or we need to read a previous lookup. Also move on (if possible) if the
    * input fifo is full. Moving on is only allowed if there is data in the
    * output_fifo, but this is always true in the first 2 conditions. */
   uint32_t section_count = 1;
   struct thread_section cur_section;

   Backflow *write_sec_start = NULL;
   Backflow *read_sec_start  = thread_count > 1 ? glsl_backflow_thrsw() : glsl_backflow_dummy();
   Backflow *first_thrsw = thread_count > 1 ? read_sec_start : NULL;

   start_new_section(&cur_section, write_sec_start, read_sec_start);

   for (struct tmu_lookup_s *lookup = block->tmu_lookups; lookup != NULL; lookup=lookup->next) {
      if ( cur_section.n_read_lookups > 0 &&
           ( cur_section.input  + lookup->write_count  > input_fifo_size  ||
             cur_section.output + lookup->read_count   > output_fifo_size ||
             cur_section.config + 1                    > config_fifo_size ||
             some_tmu_deps_in_section(lookup->last_write->tmu_deps, &cur_section)))
      {
         iodep_texture(sched_deps, read_sec_start, cur_section.pre_write);

         write_sec_start = read_sec_start;
         read_sec_start = thread_count > 1 ? glsl_backflow_thrsw() : glsl_backflow_dummy();
         iodep_texture(sched_deps, read_sec_start, cur_section.read_lookups[cur_section.n_read_lookups-1]->last_read);
         start_new_section(&cur_section, write_sec_start, read_sec_start);
         section_count++;
      }

      cur_section.input  += lookup->write_count;
      cur_section.output += lookup->read_count;
      cur_section.config += 1;

      iodep_texture(sched_deps, lookup->first_write, cur_section.pre_write);
      cur_section.pre_write = lookup->last_write;

      if (lookup->read_count > 0) {
         iodep_texture(sched_deps, lookup->first_read, cur_section.pre_read);
         cur_section.pre_read = lookup->last_read;

         cur_section.read_lookups[cur_section.n_read_lookups] = lookup;
         cur_section.n_read_lookups++;
      }
   }

   if (cur_section.n_read_lookups > 0) {
      iodep_texture(sched_deps, read_sec_start, cur_section.pre_write);
      write_sec_start = read_sec_start;
      read_sec_start = NULL;
      section_count++;
   }
   Backflow *last_thrsw = write_sec_start;

   /* It is possible for writes in this section to overfill the output fifo if
    * they are processed quickly. Loop over the writes keeping track of how much
    * is in the out fifo. Advance a read pointer whenever we're required to read
    * results back */
   struct tmu_lookup_s *read_p = NULL;
   uint32_t out_words = 0;
   for (struct tmu_lookup_s *write_p = block->tmu_lookups; write_p; write_p=write_p->next) {
      out_words += write_p->read_count;
      if (output_fifo_size < out_words) {
         while (out_words > output_fifo_size) {
            read_p = read_p ? read_p->next : block->tmu_lookups;
            assert(out_words > read_p->read_count);
            out_words -= read_p->read_count;
         }
         /* XXX This doesn't need to depend on the last read, the first for
          * which there's space in the fifo would do, but we only have the
          * first and last reads recorded. */
         iodep_texture(sched_deps, write_p->last_write, read_p->last_read);
      }
   }

   /* If the block has multiple thread sections, fix peripheral access into particular ones */
   /* TODO: Make this more intelligent */
   if (thread_count > 1) {
      if (block->first_tlb_read) {
         /* If sbwaited && section_count == 1 then there is nothing to do */
         if (sbwaited && section_count > 1) {
            /* There's already been an sbwait, so place ldtlb in the first section */
            iodep_texture(sched_deps, first_thrsw, block->last_tlb_read);
         } else if (!sbwaited) {
            /* Create a new section at the start for ldtlb. TODO: Try to use an existing one */
            Backflow *cond_false = create_sig(SIGBIT_LDUNIF);
            cond_false->unif_type = BACKEND_UNIFORM_LITERAL;
            cond_false->unif = 1;
            Backflow *fake_tmu_lookup = create_node(BACKFLOW_MOV, UNPACK_NONE, COND_IFFLAG, cond_false, cond_false, NULL, NULL);
            fake_tmu_lookup->magic_write = REG_MAGIC_TMUA;
            Backflow *fake_tmu_read = create_sig(SIGBIT_LDTMU);
            Backflow *sbwait_switch = glsl_backflow_thrsw();
            iodep_texture(sched_deps, sbwait_switch, fake_tmu_lookup);
            iodep_texture(sched_deps, fake_tmu_read, sbwait_switch);
            if (block->tmu_lookups)
               iodep_texture(sched_deps, block->tmu_lookups->first_write, fake_tmu_read);
            iodep_texture(sched_deps, block->first_tlb_read, fake_tmu_read);
            iodep_texture(sched_deps, first_thrsw, block->last_tlb_read);

            first_thrsw = sbwait_switch;
            if (last_thrsw == NULL) last_thrsw = sbwait_switch;
            section_count++;
         }
      }

      if (section_count > 1) {
         /* Force any TLB writes after the last threadswitch */
         iodep_texture(sched_deps, block->first_tlb_write, last_thrsw);

#if !V3D_HAS_LDVPM
         /* Force all vpm reads/writes into the first/last thread section */
         if (block->last_vpm_read != NULL)
            iodep_texture(sched_deps, first_thrsw, block->last_vpm_read);
         if (block->first_vpm_write != NULL)
            iodep_texture(sched_deps, block->first_vpm_write, last_thrsw);
#endif
      }
   }

   if (block->first_tlb_read && block->first_tlb_write)
      iodep_texture(sched_deps, block->first_tlb_write, block->last_tlb_read);

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

bool get_max_threadability(struct tmu_lookup_s *tmu_lookups, int *max_threads)
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

   *max_threads = max;
   return threaded;
}

GENERATED_SHADER_T *glsl_backend_schedule(SchedBlock *block,
                                          RegList *presched,
                                          RegList *outputs,
                                          int blackout,
                                          ShaderFlavour type,
                                          bool bin_mode,
                                          int threads,
                                          bool last,
                                          bool lthrsw,
                                          bool sbwaited)
{
   GENERATED_SHADER_T *result;

   BackflowPriorityQueue age_queue;
   glsl_backflow_priority_queue_init(&age_queue, 0x40000);

   struct sched_deps_s sched_deps;
   glsl_backflow_chain_init(&sched_deps.consumers);
   glsl_backflow_chain_init(&sched_deps.suppliers);

   uint32_t thrsw_count = fix_texture_dependencies(block, &sched_deps, threads, sbwaited);

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
   int branch_cond;
   if (block->branch_cond == -1) branch_cond = -1;
   else {
      branch_cond = 0;
      for (int i=block->num_outputs-1; i>block->branch_cond; i--) if (block->outputs[i] != NULL) branch_cond++;
   }
   result = glsl_backend(&block->iodeps, type, bin_mode, thrsw_count, threads, &age_queue,
                         presched, outputs, branch_cond, blackout, last, lthrsw);

#ifdef GRAPHVIZ
   graphviz_file_num++;
   graphviz(block->outputs, block->num_outputs, &block->iodeps, threads, (result != NULL));
#endif

   /* Clean up the texture iodependencies so we can try again with different threadability */
   clean_up_scheduler_iodeps(&sched_deps.consumers, &sched_deps.suppliers);

   return result;
}
