/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
//#define OUTPUT_GRAPHVIZ_ON_SUCCESS
//#define OUTPUT_GRAPHVIZ_ON_FAILURE

#include "glsl_backflow.h"
#include "glsl_backflow_visitor.h"
#include "glsl_backend.h"
#include "glsl_backend_uniforms.h"
#include "glsl_map.h"
#include "glsl_stable_sort.h"

#include "glsl_sched_node_helpers.h"
#include "glsl_sim_scheduler.h"

#include "libs/core/v3d/v3d_limits.h"

#include <stdlib.h>

#if defined(OUTPUT_GRAPHVIZ_ON_SUCCESS) || defined(OUTPUT_GRAPHVIZ_ON_FAILURE)
#define GRAPHVIZ
#include <stdio.h>
void glsl_print_backflow_from_roots(FILE *f, Backflow **roots, int num_roots, const BackflowChain *iodeps);

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
   glsl_print_backflow_from_roots(f, roots, num_roots, iodeps);
   fclose(f);
#endif
}
#endif

struct init_data {
   Map       *state_map;
   int        count;
   int        capacity;
   Backflow **nodes;
};

static void dpostv_init_backend_fields(Backflow *backend, void *data) {
   struct init_data *d = data;

   bool add_to_age_queue = backend->type != SPECIAL_VOID;

   // Don't use age to schedule following signals.
   if (backend->type == SIG) {
#if V3D_VER_AT_LEAST(4,1,34,0)
      v3d_qpu_sigbits_t sigbits = V3D_QPU_SIG_LDUNIF | V3D_QPU_SIG_LDUNIFA;
#else
      v3d_qpu_sigbits_t sigbits = V3D_QPU_SIG_LDUNIF | V3D_QPU_SIG_LDVPM;
#endif
      if (backend->u.sigbits & sigbits)
         add_to_age_queue = false;
   }

   // Don't use age to schedule writes to unifa.
#if V3D_VER_AT_LEAST(4,1,34,0)
   if (backend->magic_write == REG_MAGIC_UNIFA)
      add_to_age_queue = false;
#endif

   BackendNodeState *s = malloc_fast(sizeof(BackendNodeState));

   s->phase = PHASE_UNVISITED;
   s->reg   = REG_UNDECIDED;
   s->remaining_dependents = 0;
   glsl_backflow_chain_init(&s->data_dependents);
   for (int i = 0; i < BACKFLOW_DEP_COUNT; i++) {
      if (backend->dependencies[i] != NULL) {
         BackendNodeState *o_s = glsl_map_get(d->state_map, backend->dependencies[i]);
         o_s->remaining_dependents++;
         glsl_backflow_chain_push_back(&o_s->data_dependents, backend);
      }
   }

   glsl_map_put(d->state_map, backend, s);

   if (add_to_age_queue) {
      if (d->count == d->capacity) {
         d->capacity *= 2;
         d->nodes = glsl_safemem_realloc(d->nodes, d->capacity * sizeof(Backflow *));
      }
      d->nodes[d->count++] = backend;
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

static void iodep_texture(struct sched_deps_s *deps, Backflow *consumer, Backflow *supplier) {
   if (consumer != NULL && supplier != NULL) {
      BackflowIODep dep = {
         .dep = supplier,
         .io_timestamp_offset = 0
      };
      glsl_backflow_iodep_chain_push_back(&consumer->io_dependencies, dep);
      glsl_backflow_chain_push_back(&deps->consumers, consumer);
      glsl_backflow_chain_push_back(&deps->suppliers, supplier);
   }
}

static bool node_is_buffer(const Backflow *d) {
   return (d->type == SIG && d->u.sigbits == V3D_QPU_SIG_LDUNIF && d->unif_type == BACKEND_UNIFORM_SSBO_ADDRESS);
}

static bool node_is_shared_ptr(const Backflow *d) {
   return (d->type == SIG && d->u.sigbits == V3D_QPU_SIG_LDUNIF &&
           d->unif_type == BACKEND_UNIFORM_SPECIAL && d->unif == BACKEND_SPECIAL_UNIFORM_SHARED_PTR);
}

static Backflow *dprev_find_addr_node(Backflow *d, void *data) {
   Backflow **addr = data;
   if (*addr)
      return NULL;
   if (node_is_buffer(d) || node_is_shared_ptr(d)) {
      *addr = d;
      return NULL;
   }
   return d;
}

static Backflow *find_addr_node(Backflow *d) {
   Backflow *addr = NULL;
   BackflowVisitor *visit = glsl_backflow_visitor_begin(&addr, dprev_find_addr_node, NULL);
   glsl_backflow_visit(d, visit);
   glsl_backflow_visitor_end(visit);
   return addr;
}

void order_texture_lookups(SchedBlock *block) {
   struct tmu_lookup_s head = { .next = NULL };
   struct tmu_lookup_s *last_sorted = &head;

   if (block->tmu_lookups == NULL) return;

   Map *addr_nodes = glsl_map_new();
   for (struct tmu_lookup_s *n = block->tmu_lookups; n; n=n->next) {
      if (n->last_write->magic_write == REG_MAGIC_TMUA || n->last_write->magic_write == REG_MAGIC_TMUAU) {
         Backflow *addr_node = find_addr_node(n->last_write);
         if (addr_node) glsl_map_put(addr_nodes, n->last_write, addr_node);
      }
   }

   while (block->tmu_lookups != NULL) {
      struct tmu_lookup_s *prev = NULL;

      bool passed_buffer = false;
      struct tmu_lookup_s *best_lookup = NULL;
      struct tmu_lookup_s *best_prev   = NULL;
      for (struct tmu_lookup_s *cur = block->tmu_lookups; cur; prev=cur, cur=cur->next) {
         assert(!cur->done);   /* Done ones are no longer in the list */

         const Backflow *addr_node = glsl_map_get(addr_nodes, cur->last_write);

         /* Take this lookup next if it is ready to be scheduled, and if we don't
          * have any earlier lookup yet. If it's a write then try to move it earlier */
         if ( all_tmu_deps_done(cur->last_write->tmu_deps) &&
              (best_lookup == NULL || cur->age < best_lookup->age ||
               (best_lookup->read_count != 0 && cur->read_count == 0 && addr_node && node_is_buffer(addr_node) && !passed_buffer)))
         {
            best_prev = prev;
            best_lookup = cur;
         }

         if (addr_node && node_is_buffer(addr_node)) passed_buffer = true;
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

   glsl_map_delete(addr_nodes);
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
                                         bool lthrsw,
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
      /* TLB reads need to pick a section. We choose the first to save checking if
       * any other is OK. It also simplifies the placement below */
      if (block->first_tlb_read && section_count > 1) {
         iodep_texture(sched_deps, first_thrsw, block->last_tlb_read);
      }

      bool new_for_lthrsw = (lthrsw && last_thrsw == NULL);
      bool new_for_reads  = (block->first_tlb_read && !sbwaited);
      if (new_for_lthrsw || new_for_reads) {
#if V3D_VER_AT_LEAST(4,1,34,0)
         Backflow *new_switch = glsl_backflow_thrsw();
         Backflow *new_dep = new_switch;
#else
         Backflow *cond_false = create_sig(V3D_QPU_SIG_LDUNIF);
         cond_false->unif_type = BACKEND_UNIFORM_LITERAL;
         cond_false->unif = 1;
         Backflow *fake_tmu_lookup = tr_uop_cond(V3D_QPU_OP_MOV, COND_IFFLAG, cond_false, cond_false);
         fake_tmu_lookup->magic_write = REG_MAGIC_TMUA;
         Backflow *fake_tmu_read = create_sig(V3D_QPU_SIG_LDTMU);
         Backflow *new_switch = glsl_backflow_thrsw();
         iodep_texture(sched_deps, new_switch, fake_tmu_lookup);
         iodep_texture(sched_deps, fake_tmu_read, new_switch);
         Backflow *new_dep = fake_tmu_read;
#endif
         first_thrsw = new_switch;
         if (last_thrsw == NULL) last_thrsw = new_switch;
         section_count++;

         if (block->first_tlb_read)
            iodep_texture(sched_deps, block->first_tlb_read, new_switch);
         if (block->tmu_lookups)
            iodep_texture(sched_deps, block->tmu_lookups->first_write, new_dep);

         /* XXX Hack the dependency system. Create a dummy node which is a block
          * dependency, then create texture iodeps to the new nodes */
         /* TODO: Could we do this more directly? The sched_deps structure might just work for this */
         Backflow *dummy = glsl_backflow_dummy();
         glsl_backflow_chain_push_back(&block->iodeps, dummy);
         iodep_texture(sched_deps, dummy, new_dep);
      }

      if (section_count > 1) {
         /* Force any TLB writes after the last threadswitch */
         iodep_texture(sched_deps, block->first_tlb_write, last_thrsw);

#if !V3D_VER_AT_LEAST(4,1,34,0)
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

static void clean_up_scheduler_iodeps(BackflowChain *consumers, BackflowChain *suppliers) {

   BackflowChainNode *cnode = consumers->head;
   BackflowChainNode *snode = suppliers->head;
   while (cnode != NULL)
   {
      assert(snode != NULL);
      Backflow *consumer = cnode->val;
      Backflow *supplier = snode->val;

      BackflowIODepChainNode* n;
      for (n=consumer->io_dependencies.head; n; n=n->next) {
         if (n->val.dep == supplier) {
            glsl_backflow_iodep_chain_erase(&consumer->io_dependencies, n);
            break;
         }
      }
      assert(n != NULL); // should have removed one.

      cnode = cnode->next;
      snode = snode->next;
   }
   assert(snode == NULL);
}

#if V3D_VER_AT_LEAST(4,1,34,0)

typedef struct fix_unifa_context
{
   ldunifa_lookup* lookups;
   ldunifa_lookup* sorted_lookups;
   ldunifa_lookup** sorted_lookups_tail;
} fix_unifa_context;

static void fix_unifa_schedule_node(void* context_, Backflow* node)
{
   fix_unifa_context* context = (fix_unifa_context*)context_;

   if (node->magic_write != REG_MAGIC_UNIFA)
      return;

   // Find corresponding lookup and move to end of the sorted list.
   for (ldunifa_lookup** lookup_ptr = &context->lookups; ; ) {
      ldunifa_lookup* lookup = *lookup_ptr;
      if (!lookup)
         break;

      if (lookup->write_unifa == node) {
         *lookup_ptr = lookup->next;
         lookup->next = NULL;
         *context->sorted_lookups_tail = lookup;
         context->sorted_lookups_tail = &lookup->next;
         break;
      }
      lookup_ptr = &lookup->next;
   }

#ifndef NDEBUG
   // Assert this node is in the sorted list.
   ldunifa_lookup* lookup;
   for (lookup = context->sorted_lookups; lookup; lookup = lookup->next) {
      if (lookup->write_unifa == node)
         break;
   }
   assert(lookup != NULL);
#endif
}

static void fix_unifa_dependencies(
   SchedBlock *block,
   Backflow **sched_queue,
   unsigned sched_count,
   RegList* outputs,
   struct sched_deps_s* sched_deps)
{
   // No dependencies to fix if fewer than two ldunifa lookups.
   if (!block->ldunifa_lookups || !block->ldunifa_lookups->next)
      return;

   // Schedule all the nodes and arrange the ldunifa-lookups in order they are visited.
   fix_unifa_context context = {
      .lookups = block->ldunifa_lookups,
      .sorted_lookups = NULL,
      .sorted_lookups_tail = &context.sorted_lookups
   };

   glsl_sim_schedule_params params = {
      .schedule_node = fix_unifa_schedule_node,
      .schedule_context = &context,
      .queue = sched_queue,
      .count = sched_count,
      .outputs = outputs,
      .iodeps = &block->iodeps,
      .branch_cond = block->branch_cond
   };
   glsl_sim_schedule(&params);

   // Add unvisited lookups (can this happen?).
   *context.sorted_lookups_tail = context.lookups;

   // Assign newly sorted list back to block.
   block->ldunifa_lookups = context.sorted_lookups;

   // Create dependencies between the ldunifa lookups.
   ldunifa_lookup* prev = block->ldunifa_lookups;
   for (ldunifa_lookup* lookup = prev->next; lookup; prev = lookup, lookup = lookup->next) {
      glsl_iodep_offset(lookup->write_unifa, prev->last_ldunifa, -1); // ok to write unifa in same instruction as last ldunifa
      glsl_backflow_chain_push_back(&sched_deps->consumers, lookup->write_unifa);
      glsl_backflow_chain_push_back(&sched_deps->suppliers, prev->last_ldunifa);
   }
}

#endif

bool get_max_threadability(struct tmu_lookup_s *tmu_lookups, int *max_threads) {
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

static bool node_age_less(const void *a, const void *b)
{
   return ((const Backflow*)a)->age < ((const Backflow*)b)->age;
}

GENERATED_SHADER_T *glsl_backend_schedule(SchedBlock *block,
                                          RegList *presched,
                                          RegList *outputs,
                                          uint64_t blackout,
                                          ShaderFlavour type,
                                          bool bin_mode,
                                          int threads,
                                          bool last,
                                          bool lthrsw,
                                          bool sbwaited)
{
   struct sched_deps_s sched_deps;
   glsl_backflow_chain_init(&sched_deps.consumers);
   glsl_backflow_chain_init(&sched_deps.suppliers);

   uint32_t thrsw_count = fix_texture_dependencies(block, &sched_deps, threads, lthrsw, sbwaited);

   const size_t sched_queue_size = 2048;
   struct init_data d = { .state_map = glsl_map_new(), .count = 0,
                          .capacity = sched_queue_size,
                          .nodes = glsl_safemem_malloc(sched_queue_size * sizeof(Backflow *)) };
   BackflowVisitor *sched_visit = glsl_backflow_visitor_begin(&d, NULL, dpostv_init_backend_fields);

   /* TODO: This is messy, make it simpler and more consistent */
   for (int i=0; i<block->num_outputs; i++)
      glsl_backflow_visit(block->outputs[i], sched_visit);

   for (BackflowChainNode *n=block->iodeps.head; n; n=n->next)
      glsl_backflow_visit(n->val, sched_visit);

   if (block->branch_cond)
      glsl_backflow_visit(block->branch_cond, sched_visit);

   for (RegList *o = presched; o; o=o->next) glsl_backflow_visit(o->node, sched_visit);

   glsl_backflow_visitor_end(sched_visit);

   glsl_stable_sort((const void**)d.nodes, d.count, node_age_less);

   for (RegList *o = outputs; o; o=o->next) {
      BackendNodeState *s = glsl_map_get(d.state_map, o->node);
      s->remaining_dependents++;
   }
   if (block->branch_cond) {
      BackendNodeState *s = glsl_map_get(d.state_map, block->branch_cond);
      s->remaining_dependents++;
   }

#if V3D_VER_AT_LEAST(4,1,34,0)
   fix_unifa_dependencies(block, d.nodes, d.count, outputs, &sched_deps);
#endif

   GENERATED_SHADER_T *result = glsl_backend(&block->iodeps, type, bin_mode, thrsw_count,
                                             threads, d.nodes, d.count, presched, outputs,
                                             d.state_map, block->branch_cond, blackout, last, lthrsw);

   glsl_map_delete(d.state_map);

#ifdef GRAPHVIZ
   graphviz_file_num++;
   graphviz(block->outputs, block->num_outputs, &block->iodeps, threads, (result != NULL));
#endif

   /* Clean up the ldunifa/texture iodependencies so we can try again with different threadability */
   clean_up_scheduler_iodeps(&sched_deps.consumers, &sched_deps.suppliers);

   glsl_safemem_free(d.nodes);

   return result;
}
