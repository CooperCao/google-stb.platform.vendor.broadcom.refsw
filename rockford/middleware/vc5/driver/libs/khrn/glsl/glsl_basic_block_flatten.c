/*=============================================================================
Broadcom Proprietary and Confidential. (c)2015 Broadcom.
All rights reserved.

Project  :  glsl
Module   :

FILE DESCRIPTION
=============================================================================*/

#include "glsl_dataflow.h"
#include "glsl_basic_block.h"
#include "glsl_dataflow_simplify.h"
#include "glsl_map.h"
#include "glsl_symbols.h"
#include "glsl_primitive_types.auto.h"
#include "glsl_intern.h"
#include "glsl_shader_interfaces.h"
#include "glsl_dominators.h"

#include "libs/util/snprintf.h"

#include <stdio.h>

#define LOOP_UNROLL_ENABLED 1
#define LOOP_UNROLL_MAX_ITERATIONS 512

typedef struct {
   const Map *guard_symbols; // BasicBlock* -> Symbol*
   const Map *loops;         // BasicBlock* loop_head -> BasicBlock* loop_end_block
   const BasicBlockList *src_basic_block; // input graph (reverse-postorder)
   BasicBlock *dst_block;                 // output block
   BasicBlockList *failed_loop_unrolls;   // loop heads which could not be unrolled
   Map *block_age_offsets;                // Offset to apply to age in blocks during flattening
   bool generate_new_ids;
} flatten_context_t;

static bool copy_basic_blocks(flatten_context_t *ctx, bool is_loop_head, BasicBlock *end);

// find loops in reverse-postorder list of basic blocks
static const Map *find_loops(const BasicBlockList *basic_blocks)
{
   Map *loops = glsl_map_new(); // BasicBlock *loop_head -> BasicBlock *loop_end_block
   Map *seen = glsl_map_new();
   bool t = true;
   for (; basic_blocks; basic_blocks = basic_blocks->next) {
      const BasicBlock *basic_block = basic_blocks->v;
      glsl_map_put(seen, basic_block, &t);
      if (basic_block->fallthrough_target && glsl_map_get(seen, basic_block->fallthrough_target))
         glsl_map_put(loops, basic_block->fallthrough_target, basic_blocks->next->v);
      if (basic_block->branch_target && glsl_map_get(seen, basic_block->branch_target))
         glsl_map_put(loops, basic_block->branch_target, basic_blocks->next->v);
   }
   return loops;
}

// create guard symbols for predicated basic blocks. Blocks with no guard
// symbols are unconditional. They always execute when encountered.
static const Map *create_guards(const BasicBlockList *basic_blocks)
{
   Map *guard_symbols = glsl_map_new();
   bool *uncond = glsl_safemem_malloc(glsl_basic_block_list_count(basic_blocks) * sizeof(bool));
   glsl_find_unconditional_blocks(basic_blocks, uncond);

   int id = 0;
   for (const BasicBlockList *node = basic_blocks; node; node = node->next, id++) {
      if (uncond[id]) continue;

      const BasicBlock *basic_block = node->v;
      Symbol *guard_symbol = glsl_symbol_construct_temporary(&primitiveTypes[PRIM_BOOL]);
      char name[64];
      snprintf(name, 64, "$guard$%i", guard_symbols->count);
      guard_symbol->name = glsl_intern(name, true);
      glsl_map_put(guard_symbols, basic_block, guard_symbol);
   }

   glsl_safemem_free(uncond);
   return guard_symbols;
}

// make a copy of the dataflow and costant fold it
static Dataflow *dataflow_simplify_recursive(Map *dataflow_copy, int age_offset, Dataflow *original, bool copy)
{
   if (original == NULL) return NULL;

   Dataflow *n = glsl_map_get(dataflow_copy, original);
   if (n) return n;

   if (copy) {
      n = malloc_fast(sizeof(Dataflow));
      memcpy(n, original, sizeof(Dataflow));
      n->id = glsl_dataflow_get_next_id();
   } else
      n = original;

   for (int i = 0; i < original->dependencies_count; i++)
      n->d.dependencies[i] = dataflow_simplify_recursive(dataflow_copy, age_offset, original->d.dependencies[i], copy);

   n->age = original->age + age_offset;

   Dataflow *simplified = glsl_dataflow_simplify(n);
   if (simplified != NULL) {
      simplified->age = n->age;
      n = simplified;
   }

   glsl_map_put(dataflow_copy, original, n);
   return n;
}

static bool guard_is_always_true(Dataflow *guard_value) {
   return guard_value == NULL ||
          (guard_value->flavour == DATAFLOW_CONST && guard_value->u.constant.value == 1);
}

// set scalar values in the dst_basic_block
static void set_dst_scalar_values(BasicBlock *dst_block, const BasicBlock *src_block,
                                  const Symbol *symbol, Dataflow **new_scalar_values, Dataflow *guard)
{
   bool is_uniform = (symbol->flavour == SYMBOL_INTERFACE_BLOCK || (symbol->flavour == SYMBOL_VAR_INSTANCE && symbol->u.var_instance.storage_qual == STORAGE_UNIFORM));

   if (is_uniform || guard_is_always_true(guard)) {
      glsl_basic_block_set_scalar_values(dst_block, symbol, new_scalar_values);
   } else {
      Dataflow **v = glsl_basic_block_get_scalar_values(dst_block, symbol);
      for (unsigned i = 0; i < symbol->type->scalar_count; i++) {
         if (new_scalar_values[i]->type == DF_FSAMPLER ||
             new_scalar_values[i]->type == DF_ISAMPLER ||
             new_scalar_values[i]->type == DF_USAMPLER ||
             new_scalar_values[i]->type == DF_FIMAGE   ||
             new_scalar_values[i]->type == DF_IIMAGE   ||
             new_scalar_values[i]->type == DF_UIMAGE)
         {
            /* In addition to uniforms (caught above) we can hit this case for param instances */
            v[i] = new_scalar_values[i];
         }
         else
            v[i] = glsl_dataflow_construct_ternary_op(DATAFLOW_CONDITIONAL, guard, new_scalar_values[i], v[i]);
      }
      glsl_basic_block_set_scalar_values(dst_block, symbol, v);
   }
}

static void copy_block_dataflow(BasicBlock *dst_block, const BasicBlock *src_block, Dataflow *guard_value,
                                Map *age_offsets, bool generate_new_ids, Dataflow **branch_cond, Dataflow **not_branch_cond)
{
   int *src_last_age = glsl_map_get(age_offsets, src_block);
   int *dst_last_age = glsl_map_get(age_offsets, dst_block);
   int age_offset = *dst_last_age;
   *dst_last_age += *src_last_age;

   Map *dataflow_copy = glsl_map_new(); // Dataflow* -> Dataflow*
   // Prepare constant folding - bind dataflow inputs to actual values
   for (MapNode *load = src_block->loads->head; load; load = load->next) {
      const Symbol *symbol = load->k;
      Dataflow **load_scalar_values = load->v;
      Dataflow **actual_scalar_values = glsl_basic_block_get_scalar_values(dst_block, symbol);

      for (unsigned i = 0; i < symbol->type->scalar_count; i++) {
         assert(load_scalar_values[i]->flavour == DATAFLOW_LOAD);
         glsl_map_put(dataflow_copy, load_scalar_values[i], actual_scalar_values[i]);
      }
   }

   /* Place guard dataflow at the end of the blocks age range */
   glsl_dataflow_set_age(age_offset);

   for (MapNode *node = src_block->scalar_values->head; node; node = node->next) {
      const Symbol *symbol = node->k;
      Dataflow **scalar_values = node->v;
      Dataflow **simplified_values = glsl_safemem_malloc(sizeof(Dataflow*) * symbol->type->scalar_count);

      for (unsigned i = 0; i < symbol->type->scalar_count; i++)
         simplified_values[i] = dataflow_simplify_recursive(dataflow_copy, age_offset, scalar_values[i], generate_new_ids);

      set_dst_scalar_values(dst_block, src_block, symbol, simplified_values, guard_value);

      glsl_safemem_free(simplified_values);
   }

   Dataflow *memory_head = dataflow_simplify_recursive(dataflow_copy, age_offset,
                                                       src_block->memory_head, generate_new_ids);
   for (Dataflow *m = memory_head; m != NULL; m=m->d.addr_store.prev) {
      if (!guard_is_always_true(guard_value)) {
         if (m->d.addr_store.cond != NULL) m->d.addr_store.cond = glsl_dataflow_construct_binary_op(DATAFLOW_LOGICAL_AND, m->d.addr_store.cond, guard_value);
         else m->d.addr_store.cond = guard_value;
      }
      if (m->d.addr_store.prev == NULL) {
         m->d.addr_store.prev = dst_block->memory_head;
         dst_block->memory_head = memory_head;
         break;
      }
   }

   if (src_block->branch_cond) {
      *branch_cond = dataflow_simplify_recursive(dataflow_copy, age_offset, src_block->branch_cond, generate_new_ids);
      *not_branch_cond = glsl_dataflow_construct_unary_op(DATAFLOW_LOGICAL_NOT, *branch_cond);
   } else {
      *branch_cond = NULL;
      *not_branch_cond = glsl_dataflow_construct_const_value(DF_BOOL, true);
   }
}

// copy dataflow from src_basic_block++ to dst_basic_block
static void copy_basic_block(BasicBlock *dst_block, const BasicBlock *src_block,
                             const Map *guard_symbols, Map *age_offsets, bool generate_new_ids)
{
   // Load our guard
   Symbol *guard_symbol = glsl_map_get(guard_symbols, src_block);
   Dataflow *guard_value = NULL;    /* NULL is interpreted as always true */
   if (guard_symbol != NULL) {
      guard_value = glsl_basic_block_get_scalar_values(dst_block, guard_symbol)[0];
      /* Early out if this block isn't executing at all */
      if (guard_value->flavour == DATAFLOW_CONST && guard_value->u.constant.value == 0) return;
   }

   Dataflow *branch_cond, *not_branch_cond;
   copy_block_dataflow(dst_block, src_block, guard_value, age_offsets, generate_new_ids, &branch_cond, &not_branch_cond);

   // Clear our guard at the end of the basic block
   if (guard_symbol != NULL) {
      Dataflow *f = glsl_dataflow_construct_const_bool(false);
      set_dst_scalar_values(dst_block, src_block, guard_symbol, &f, NULL);
   }

   // Pass the guard to the successor which is to be executed next
   if (src_block->branch_target) {
      const Symbol *guard_symbol = glsl_map_get(guard_symbols, src_block->branch_target);
      if (guard_symbol != NULL)
         set_dst_scalar_values(dst_block, src_block, guard_symbol, &branch_cond, guard_value);
   }
   if (src_block->fallthrough_target) {
      const Symbol *guard_symbol = glsl_map_get(guard_symbols, src_block->fallthrough_target);
      if (guard_symbol != NULL) {
         set_dst_scalar_values(dst_block, src_block, guard_symbol, &not_branch_cond, guard_value);
      }
   }
}

// unroll a loop by copying all the blocks and flattening
static bool copy_static_loop(flatten_context_t *ctx, BasicBlock *src_loop_end_block)
{
   const BasicBlockList *src_loop_head = ctx->src_basic_block;
   Symbol *loop_head_guard = glsl_map_get(ctx->guard_symbols, src_loop_head->v);
   ctx->generate_new_ids = true;
   for (int iteration = 0; iteration < LOOP_UNROLL_MAX_ITERATIONS; iteration++) {
      Dataflow *guard_value;
      if (!copy_basic_blocks(ctx, true, src_loop_end_block))
         return false;

      guard_value = glsl_basic_block_get_scalar_value(ctx->dst_block, loop_head_guard, 0);
      if (guard_value->flavour == DATAFLOW_CONST && guard_value->u.constant.value == 0)
         return true; // unroll succeeded

      ctx->src_basic_block = src_loop_head;
   }
   // unroll failed
   glsl_basic_block_list_add(&ctx->failed_loop_unrolls, src_loop_head->v);
   return false;
}

// copy a loop
static bool copy_dynamic_loop(flatten_context_t *ctx, BasicBlock *src_loop_end_block)
{
   const BasicBlock *src_loop_head = ctx->src_basic_block->v;

   BasicBlock *loop_head = glsl_basic_block_construct();
   BasicBlock *end_block = glsl_basic_block_construct();
   int *head_age = malloc_fast(sizeof(int));
   int *end_age = malloc_fast(sizeof(int));
   *head_age = *end_age = 0;
   glsl_map_put(ctx->block_age_offsets, loop_head, head_age);

   // start new basic block
   ctx->dst_block->branch_target = NULL;
   ctx->dst_block->fallthrough_target = loop_head;
   ctx->dst_block = loop_head;

   if (!copy_basic_blocks(ctx, true, src_loop_end_block))
      return false;

   glsl_map_put(ctx->block_age_offsets, end_block, end_age);

   // start new basic block
   Symbol *loop_head_guard = glsl_map_get(ctx->guard_symbols, src_loop_head);
   ctx->dst_block->branch_cond = glsl_basic_block_get_scalar_value(ctx->dst_block, loop_head_guard, 0);
   ctx->dst_block->branch_target = loop_head;
   ctx->dst_block->fallthrough_target = end_block;
   ctx->dst_block = end_block;

   return true;
}

// copy range of basic blocks
static bool copy_basic_blocks(flatten_context_t *ctx, bool is_loop_head, BasicBlock *end)
{
   while (ctx->src_basic_block && ctx->src_basic_block->v != end) {
      BasicBlock *src_loop_head = ctx->src_basic_block->v;
      BasicBlock *src_loop_end_block = glsl_map_get(ctx->loops, src_loop_head);
      // recurse into loop (unless we are already being called from that loop)
      if (src_loop_end_block && !is_loop_head) {
         if (!glsl_basic_block_list_contains(ctx->failed_loop_unrolls, src_loop_head) && LOOP_UNROLL_ENABLED) {
            if (!copy_static_loop(ctx, src_loop_end_block))
               return false;
         } else {
            if (!copy_dynamic_loop(ctx, src_loop_end_block))
               return false;
         }
      } else {
         copy_basic_block(ctx->dst_block, ctx->src_basic_block->v,
                          ctx->guard_symbols, ctx->block_age_offsets, ctx->generate_new_ids);
         if (ctx->src_basic_block->v->barrier) {
            ctx->dst_block->barrier = true;
            int *age = malloc_fast(sizeof(int));
            *age = 0;
            BasicBlock *d = glsl_basic_block_construct();
            glsl_map_put(ctx->block_age_offsets, d, age);
            ctx->dst_block->fallthrough_target = d;
            ctx->dst_block = d;
         }
         ctx->src_basic_block = ctx->src_basic_block->next;
      }
      is_loop_head = false;
   }
   return true;
}

BasicBlock *glsl_basic_block_flatten(BasicBlock *entry, Map *block_age_offsets)
{
   flatten_context_t ctx;
   const BasicBlockList *basic_blocks = glsl_basic_block_get_reverse_postorder_list(entry);
   BasicBlock *copy;

   ctx.guard_symbols = create_guards(basic_blocks);
   ctx.loops = find_loops(basic_blocks);
   ctx.failed_loop_unrolls = NULL;
   ctx.block_age_offsets = block_age_offsets;
   ctx.generate_new_ids = true;

   do {
      ctx.src_basic_block = basic_blocks;
      copy = glsl_basic_block_construct();
      ctx.dst_block = copy;
      int *dst_age = malloc_fast(sizeof(int));
      *dst_age = 0;
      glsl_map_put(block_age_offsets, ctx.dst_block, dst_age);

      for (MapNode *map_node = ctx.guard_symbols->head; map_node; map_node = map_node->next) {
         const Symbol *symbol = map_node->v;
         Dataflow *f = glsl_dataflow_construct_const_bool(false);
         glsl_basic_block_set_scalar_values(ctx.dst_block, symbol, &f);
      }

      /* We know the entry block is unconditional, so has no guard */
      assert(glsl_map_get(ctx.guard_symbols, entry) == NULL);
   } while (!copy_basic_blocks(&ctx, false, NULL));

   return copy;
}








static void update_guard_values(Dataflow **guard, Dataflow *new_scalar_value, Dataflow *guard_val)
{
   if (guard_val != NULL)
      guard[0] = glsl_dataflow_construct_ternary_op(DATAFLOW_CONDITIONAL, guard_val, new_scalar_value, guard[0]);
   else
      guard[0] = new_scalar_value;
}

static void update_initial_guards(BasicBlock *block, Map *guards)
{
   if (!block->branch_target) return;

   // Pass the guard to the successor which is to be executed next
   Dataflow **branch_guard      = glsl_map_get(guards, block->branch_target);
   Dataflow **fallthrough_guard = glsl_map_get(guards, block->fallthrough_target);

   glsl_dataflow_set_age(0);
   Dataflow *branch_cond = block->branch_cond;
   Dataflow *not_branch_cond = glsl_dataflow_construct_unary_op(DATAFLOW_LOGICAL_NOT, branch_cond);

   if (branch_guard      != NULL) update_guard_values(branch_guard, branch_cond, NULL);
   if (fallthrough_guard != NULL) update_guard_values(fallthrough_guard, not_branch_cond, NULL);
}

static void copy_basic_block_simple(BasicBlock *dst_block, const BasicBlock *src_block, Map *age_offsets, Map *guards, bool last)
{
   // Load our guard
   Dataflow **guard = glsl_map_get(guards, src_block);
   Dataflow *guard_value = NULL;
   if (guard != NULL)
      guard_value = guard[0];
   if (guard_value && guard_value->flavour == DATAFLOW_CONST && guard_value->u.constant.value == 0) return;

   Dataflow *branch_cond, *not_branch_cond;
   copy_block_dataflow(dst_block, src_block, guard_value, age_offsets, false, &branch_cond, &not_branch_cond);

   // Pass the guard to the successor which is to be executed next
   if (src_block->branch_target) {
      Dataflow **guard = glsl_map_get(guards, src_block->branch_target);
      if (guard != NULL) update_guard_values(guard, branch_cond, guard_value);
   }
   if (src_block->fallthrough_target) {
      Dataflow **guard = glsl_map_get(guards, src_block->fallthrough_target);
      if (guard != NULL) update_guard_values(guard, not_branch_cond, guard_value);
   }

   if (last) dst_block->branch_cond = branch_cond;
}

static Map *create_guard_vals(const BasicBlockList *basic_blocks)
{
   Map *guard_vals = glsl_map_new();
   bool *uncond = glsl_safemem_malloc(glsl_basic_block_list_count(basic_blocks) * sizeof(bool));
   glsl_find_unconditional_blocks(basic_blocks, uncond);

   int id = 0;
   for (const BasicBlockList *node = basic_blocks; node; node = node->next, id++) {
      if (uncond[id]) continue;

      const BasicBlock *basic_block = node->v;
      Dataflow **gv = glsl_safemem_malloc(sizeof(Dataflow *));
      glsl_map_put(guard_vals, basic_block, gv);
   }

   glsl_safemem_free(uncond);
   return guard_vals;
}

void flatten_into(BasicBlock *entry, Map *block_age_offsets) {
   const BasicBlockList *basic_blocks = glsl_basic_block_get_reverse_postorder_list(entry);
   Map *guards = create_guard_vals(basic_blocks);
   /* We know the entry block is unconditional, so has no guard */
   assert(glsl_map_get(guards, entry) == NULL);

   for (MapNode *map_node = guards->head; map_node; map_node = map_node->next) {
      Dataflow **gv = map_node->v;
      gv[0] = glsl_dataflow_construct_const_bool(false);
   }

   update_initial_guards(entry, guards);
   basic_blocks = basic_blocks->next;

   while (basic_blocks) {
      copy_basic_block_simple(entry, basic_blocks->v, block_age_offsets, guards, (basic_blocks->next == NULL));
      basic_blocks = basic_blocks->next;
   }

   for (MapNode *n = guards->head; n; n=n->next) glsl_safemem_free(n->v);
}

bool glsl_basic_block_flatten_a_bit(BasicBlock *entry, Map *block_age_offsets) {
   BasicBlockList *l = glsl_basic_block_get_reverse_postorder_list(entry);
   int head, exit;

   bool merge = glsl_ssa_block_flatten(l, &head, &exit);
   if (!merge) return false;

   BasicBlock *b_head = NULL, *b_exit;
   int i;
   BasicBlockList *n;
   for (i=0, n=l; i < exit; i++, n=n->next) {
      if (i == head) b_head = n->v;
   }
   assert(b_head != NULL);
   b_exit = n->v;

   /* TODO: Further, we give up folding if any block here contains a barrier */
   bool active = false;
   for (BasicBlockList *n = l; n; n=n->next) {
      if (n->v == b_head) active = true;
      if (n->v == b_exit) break;
      if (active && n->v->barrier) return false;
   }

   BasicBlock *branch = b_exit->branch_target;
   BasicBlock *fallthrough = b_exit->fallthrough_target;
   b_exit->branch_target = NULL;
   b_exit->fallthrough_target = NULL;

   flatten_into(b_head, block_age_offsets);

   b_head->branch_target = branch;
   b_head->fallthrough_target = fallthrough;

   return true;
}
