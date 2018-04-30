/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <setjmp.h>

#include "glsl_common.h"
#include "glsl_fastmem.h"
#include "glsl_errors.h"
#include "glsl_compiler.h"
#include "glsl_parser.h"
#include "glsl_map.h"
#include "glsl_globals.h"
#include "glsl_symbols.h"
#include "glsl_ast.h"
#include "glsl_ast_visitor.h"
#include "glsl_nast_builder.h"
#include "glsl_dataflow.h"
#include "glsl_dataflow_simplify.h"
#include "glsl_shader_interfaces.h"
#include "glsl_source.h"
#include "glsl_basic_block_builder.h"
#include "glsl_basic_block_elim_dead.h"
#include "glsl_basic_block_flatten.h"
#include "glsl_basic_block_print.h"
#include "glsl_dominators.h"
#include "glsl_stringbuilder.h"
#include "glsl_intern.h"
#include "glsl_dataflow_visitor.h"
#include "glsl_mem_layout.h"
#include "glsl_ir_program.h"
#include "glsl_dataflow_builder.h"
#include "glsl_stackmem.h"
#include "glsl_dataflow_cse.h"
#include "glsl_layout.h"
#include "glsl_compiled_shader.h"
#include "glsl_extensions.h"
#include "glsl_ssa_convert.h"

#include "glsl_primitive_types.auto.h"
#include "glsl_stdlib.auto.h"

#include "../glxx/glxx_int_config.h"

#include "libs/util/gfx_util/gfx_util.h"
#include "libs/core/v3d/v3d_gen.h"

void glsl_term_lexer();
bool glsl_find_version(int sourcec, const char *const *sourcev, int *version);

CompiledShader *glsl_compiled_shader_create(ShaderFlavour f, int version) {
   CompiledShader *ret = malloc(sizeof(CompiledShader));
   if (ret == NULL) return NULL;

   ret->flavour = f;
   ret->version = version;

   ret->str_block           = NULL;
   ret->type_block          = NULL;
   ret->lq_block            = NULL;
   ret->symbol_block        = NULL;
   ret->struct_member_block = NULL;

   glsl_ir_shader_init(&ret->ir);

   ret->uniform.n_vars = 0;
   ret->uniform.var    = NULL;
   ret->in.n_vars      = 0;
   ret->in.var         = NULL;
   ret->out.n_vars     = 0;
   ret->out.var        = NULL;
   ret->buffer.n_vars  = 0;
   ret->buffer.var     = NULL;
   return ret;
}

static void free_shader_interface(ShaderInterface* iface) {
   for (unsigned i = 0; i != iface->n_vars; i++) {
      free(iface->var[i].ids);
#if V3D_VER_AT_LEAST(4,1,34,0)
      free(iface->var[i].flags);
#endif
   }
   free(iface->var);
}

void glsl_compiled_shader_free(CompiledShader *sh) {
   if (sh == NULL) return;

   free(sh->str_block);
   free(sh->type_block);
   free(sh->lq_block);
   free(sh->symbol_block);
   free(sh->struct_member_block);

   glsl_ir_shader_term(&sh->ir);

   free_shader_interface(&sh->uniform);
   free_shader_interface(&sh->in);
   free_shader_interface(&sh->out);
   free_shader_interface(&sh->buffer);
   free(sh);
}

static int symbol_list_count(SymbolList *l) {
   int i;
   SymbolListNode *n;
   for (i=0, n = l->head; n != NULL; n = n->next, i++);
   return i;
}

static bool interface_from_symbols(ShaderInterface *iface, const struct if_usage *l,
                                   Map *symbol_map, Map *symbol_ids, bool alloc_flags)
{
   bool out_of_memory = false;

   iface->n_vars = l->n;
   iface->var    = malloc(l->n * sizeof(InterfaceVar));
   if (iface->var == NULL) {
      iface->n_vars = 0;
      return false;
   }

   for (unsigned i=0; i<l->n; i++) {
      InterfaceVar *v = &iface->var[i];

      v->symbol = glsl_map_get(symbol_map, l->v[i].symbol);
      v->active = false;
#if V3D_VER_AT_LEAST(4,1,34,0)
      v->flags = NULL;
#endif

      v->static_use = l->v[i].used;
      v->ids = malloc(l->v[i].symbol->type->scalar_count * sizeof(int));
      if (v->ids == NULL) {
         out_of_memory = true;
         continue;      /* We'll free all of these, continue ensures they are initialised */
      }

#if V3D_VER_AT_LEAST(4,1,34,0)
      if (alloc_flags) {
         v->flags = calloc(l->v[i].symbol->type->scalar_count, sizeof(InterfaceVarFlags));
         if (v->flags == NULL) {
            out_of_memory = true;
            continue;
         }
      }
#endif

      for (unsigned j=0; j<l->v[i].symbol->type->scalar_count; j++) {
         v->ids[j] = -1;
      }

      int *ids = glsl_map_get(symbol_ids, l->v[i].symbol);
      if (ids) {
         for (unsigned j=0; j<l->v[i].symbol->type->scalar_count; j++)
            v->ids[j] = ids[j];

         /* Conservatively assume that anything statically used might be active.
          * For everything except outputs this will be refined later */
         v->active = v->static_use;
      }
   }

   return !out_of_memory;
}

static void find_static_use(Expr *e, void *data) {
   struct if_usage *d = data;

   if (e->flavour == EXPR_INSTANCE) {
      assert(e->u.instance.symbol != NULL);
      for (unsigned i=0; i<d->n; i++) {
         if (e->u.instance.symbol == d->v[i].symbol) d->v[i].used = true;
         if (e->u.instance.symbol->flavour == SYMBOL_VAR_INSTANCE &&
             e->u.instance.symbol->u.var_instance.block_info_valid &&
             e->u.instance.symbol->u.var_instance.block_info.block_symbol == d->v[i].symbol)
         {
            d->v[i].used = true;
         }
      }
   }
}

static void if_symbols_from_symbol_list(struct if_usage *i, SymbolList *l, Statement *ast) {
   i->n = symbol_list_count(l);
   i->v = glsl_safemem_malloc(i->n * sizeof(struct symbol_usage));

   unsigned s;
   SymbolListNode *n;
   for (s=0, n=l->head; s<i->n; s++, n=n->next) {
      i->v[s].symbol = n->s;
      i->v[s].used   = false;
   }

   glsl_statement_accept_postfix(ast, i, NULL, find_static_use);
}

InterfaceVar *interface_var_find(ShaderInterface *i, const char *name) {
   for (int j=0; j<i->n_vars; j++) {
      if (!strcmp(name, i->var[j].symbol->name)) return &i->var[j];
   }
   return NULL;
}

int interface_max_id(const ShaderInterface *i) {
   int max = -1;
   for (int j=0; j<i->n_vars; j++) {
      for (unsigned k=0; k<i->var[j].symbol->type->scalar_count; k++) {
         if (i->var[j].ids[k] > max) max = i->var[j].ids[k];
      }
   }
   return max;
}

struct id_actives {
   bool *in;
   bool *uniform;
   bool *buffer;
   uint32_t atomic[GLXX_CONFIG_MAX_ATOMIC_COUNTER_BUFFER_BINDINGS];
};

static void dpostv_find_active_ids(Dataflow *dataflow, void *data) {
   struct id_actives *active = data;
   switch (dataflow->flavour) {
      case DATAFLOW_IN:
         active->in[dataflow->u.linkable_value.row] = true;
         break;
      case DATAFLOW_UNIFORM:
         active->uniform[dataflow->u.linkable_value.row] = true;
         break;
      case DATAFLOW_UNIFORM_BUFFER:
         active->uniform[dataflow->u.linkable_value.row] = true;
         break;
      case DATAFLOW_STORAGE_BUFFER:
         active->buffer[dataflow->u.linkable_value.row] = true;
         break;
      case DATAFLOW_CONST_IMAGE:
         active->uniform[dataflow->u.const_image.location] = true;
         break;
      case DATAFLOW_CONST_SAMPLER:
         active->uniform[dataflow->u.linkable_value.row] = true;
         break;
      case DATAFLOW_ATOMIC_COUNTER:
         active->atomic[dataflow->u.buffer.index] |= (1 << (dataflow->u.buffer.offset / 4));
         break;
      default:
         break;
   }
}

static SymbolType *base_type(SymbolType *array_type) {
   while (array_type->flavour == SYMBOL_ARRAY_TYPE) array_type = array_type->u.array_type.member_type;
   return array_type;
}

static void update_interface(ShaderInterface *iface, bool *actives) {
   for (int i=0; i<iface->n_vars; i++) {
      InterfaceVar *v = &iface->var[i];
      bool active = false;
      for (unsigned j=0; j<v->symbol->type->scalar_count; j++) {
         if (v->ids[j] != -1 && !actives[v->ids[j]])
            v->ids[j] = -1;

         if (v->ids[j] != -1)
            active = true;
      }
      v->active = active;
   }
}

static void update_atomics(ShaderInterface *iface, uint32_t *actives) {
   for (int i=0; i<iface->n_vars; i++) {
      InterfaceVar *v = &iface->var[i];
      if (glsl_prim_is_prim_atomic_type(base_type(v->symbol->type))) {
         int binding = v->symbol->u.var_instance.layout_binding;
         int counter_base = v->symbol->u.var_instance.offset / 4;
         int num_counters = v->symbol->type->scalar_count;
         int counter_mask = gfx_mask(counter_base + num_counters) & ~gfx_mask(counter_base);
         if (actives[binding] & counter_mask) v->active = true;
      }
   }
}

#if V3D_VER_AT_LEAST(4,1,34,0)

static void dpostv_detect_dynamic_indexed_samplers(Dataflow *dataflow, void *data)
{
   if (dataflow->flavour != DATAFLOW_SAMPLER_UNNORMS)
      return;

   Dataflow *sampler = dataflow->d.unary_op.operand;
   assert(sampler->flavour == DATAFLOW_CONST_SAMPLER);
   assert(sampler->type    == DF_SAMPLER);
   unsigned loc = sampler->u.linkable_value.row;
   ((bool*)data)[loc] = true;
}

static void detect_dynamic_indexed_samplers(ShaderInterface *uniform, const SSABlock *blocks, unsigned n_blocks)
{
   bool *sampler_dynamic_indexing = glsl_safemem_calloc(interface_max_id(uniform)+1, sizeof(bool));

   for (unsigned i = 0; i != n_blocks; ++i) {
      glsl_dataflow_visit_array(blocks[i].outputs, 0, blocks[i].n_outputs,
         sampler_dynamic_indexing, NULL, dpostv_detect_dynamic_indexed_samplers);
   }

   for (unsigned i = 0; i != uniform->n_vars; ++i) {
      InterfaceVar *v = &uniform->var[i];
      bool all_zero = true;
      if (v->active) {
         for (unsigned j = 0; j != v->symbol->type->scalar_count; ++j) {
            if (v->ids[j] != -1 && sampler_dynamic_indexing[v->ids[j]]) {
               v->flags[j] |= INTERFACE_VAR_SAMPLER_DYNAMIC_INDEXING;
               all_zero = false;
            }
         }
      }

      // Throw out flags array if empty.
      if (all_zero) {
         free(v->flags);
         v->flags = NULL;
      }
   }

   glsl_safemem_free(sampler_dynamic_indexing);
}

#endif

static void mark_interface_actives(ShaderInterface *in, ShaderInterface *uniform, ShaderInterface *buffer,
                                   const SSABlock *block, int n_blocks)
{
   struct id_actives active;
   active.in      = glsl_safemem_calloc(interface_max_id(in)+1,      sizeof(bool));
   active.uniform = glsl_safemem_calloc(interface_max_id(uniform)+1, sizeof(bool));
   active.buffer  = glsl_safemem_calloc(interface_max_id(buffer)+1,  sizeof(bool));
   for (int i=0; i<GLXX_CONFIG_MAX_ATOMIC_COUNTER_BUFFER_BINDINGS; i++) active.atomic[i] = 0;

   for (int i=0; i<n_blocks; i++)
      glsl_dataflow_visit_array(block[i].outputs, 0, block[i].n_outputs, &active, NULL, dpostv_find_active_ids);

   update_interface(in,      active.in);
   update_interface(uniform, active.uniform);
   update_interface(buffer,  active.buffer);

   update_atomics(uniform, active.atomic);

#if V3D_VER_AT_LEAST(4,1,34,0)
   detect_dynamic_indexed_samplers(uniform, block, n_blocks);
#endif

   glsl_safemem_free(active.in);
   glsl_safemem_free(active.uniform);
   glsl_safemem_free(active.buffer);
}

struct shader_maps {
   Map *string;
   Map *type;
   Map *symbol;
   Map *lq;
   Map *struct_members;
};

static void shader_maps_init(struct shader_maps *maps) {
   maps->string         = glsl_map_new();
   maps->type           = glsl_map_new();
   maps->symbol         = glsl_map_new();
   maps->lq             = glsl_map_new();
   maps->struct_members = glsl_map_new();
}

static void shader_maps_term(struct shader_maps *maps) {
   glsl_map_delete(maps->string);
   glsl_map_delete(maps->type);
   glsl_map_delete(maps->symbol);
   glsl_map_delete(maps->lq);
   glsl_map_delete(maps->struct_members);
}

static void record_type(struct shader_maps *m, SymbolType *t);

static void record_string(struct shader_maps *m, const char *s) {
   if (glsl_map_get(m->string, s) != NULL) return;
   glsl_map_put(m->string, s, (void *)0x1);
}

static void record_lq(struct shader_maps *m, LayoutQualifier *lq) {
   if (glsl_map_get(m->lq, lq) != NULL) return;
   glsl_map_put(m->lq, lq, lq);
}

static void record_struct_members(struct shader_maps *m, StructMember *member, unsigned member_count) {
   if (glsl_map_get(m->struct_members, member) != NULL) return;
   glsl_map_put(m->struct_members, member, (void *)(uintptr_t)member_count);

   for (unsigned i=0; i<member_count; i++) {
      if (member[i].layout != NULL) record_lq(m, member[i].layout);
      record_string(m, member[i].name);
      record_type(m, member[i].type);
   }
}

static void record_type(struct shader_maps *m, SymbolType *t) {
   if (glsl_map_get(m->type, t) != NULL) return;

   glsl_map_put(m->type, t, t);
   record_string(m, t->name);

   switch(t->flavour) {
      case SYMBOL_PRIMITIVE_TYPE:
         return;
      case SYMBOL_ARRAY_TYPE:
         record_type(m, t->u.array_type.member_type);
         return;
      case SYMBOL_STRUCT_TYPE:
         record_struct_members(m, t->u.struct_type.member, t->u.struct_type.member_count);
         return;
      case SYMBOL_BLOCK_TYPE:
         record_struct_members(m, t->u.block_type.member, t->u.block_type.member_count);
         if (t->u.block_type.lq)
            record_lq(m, t->u.block_type.lq);
         return;
      default: unreachable();
   }
}

static void record_symbol(struct shader_maps *m, const Symbol *s) {
   if (glsl_map_get(m->symbol, s) != NULL) return;

   glsl_map_put(m->symbol, s, (Symbol *)s); /* XXX Cast away const. This value is ignored */

   record_string(m, s->name);
   record_type(m, s->type);

   if (s->flavour == SYMBOL_INTERFACE_BLOCK) record_type(m, s->u.interface_block.block_data_type);
}

static void record_interface(struct shader_maps *m, const struct if_usage *iface) {
   for (unsigned i=0; i<iface->n; i++)
      record_symbol(m, iface->v[i].symbol);
}

static bool copy_compiled_shader(CompiledShader *sh, const struct sh_usage *ifaces, Map *symbol_map) {
   struct shader_maps maps;
   struct shader_maps output;

   shader_maps_init(&maps);

   record_interface(&maps, &ifaces->uniform);
   record_interface(&maps, &ifaces->in);
   record_interface(&maps, &ifaces->out);
   record_interface(&maps, &ifaces->buffer);

   shader_maps_init(&output);

   uintptr_t string_idx = 1;
   GLSL_MAP_FOREACH(e, maps.string) {
      const char *str = e->k;
      glsl_map_put(output.string, str, (void *)string_idx);
      string_idx += strlen(str) + 1;
   }

   uintptr_t type_idx = 1;
   GLSL_MAP_FOREACH(e, maps.type) {
      glsl_map_put(output.type, e->k, (void *)type_idx);
      type_idx++;
   }

   uintptr_t lq_idx = 1;
   GLSL_MAP_FOREACH(e, maps.lq) {
      glsl_map_put(output.lq, e->k, (void *)lq_idx);
      lq_idx++;
   }

   uintptr_t symbol_idx = 1;
   GLSL_MAP_FOREACH(e, maps.symbol) {
      glsl_map_put(output.symbol, e->k, (void *)symbol_idx);
      symbol_idx++;
   }

   /* XXX This one can start from 0 because we're using map properly */
   int struct_member_idx = 0;
   GLSL_MAP_FOREACH(e, maps.struct_members) {
      uintptr_t arr_len = (uintptr_t)e->v;
      int *val = malloc_fast(2*sizeof(int));
      val[0] = arr_len;
      val[1] = struct_member_idx;
      glsl_map_put(output.struct_members, e->k, val);
      struct_member_idx += arr_len;
   }

   shader_maps_term(&maps);

   /* We're ready to start copying now. Allocate memory blocks */
   sh->str_block  = malloc(string_idx * sizeof(char));
   sh->type_block = malloc(type_idx   * sizeof(SymbolType));
   sh->lq_block   = malloc(lq_idx     * sizeof(LayoutQualifier));
   sh->symbol_block        = malloc(symbol_idx        * sizeof(Symbol));
   sh->struct_member_block = malloc(struct_member_idx * sizeof(StructMember));
   if (!sh->str_block || !sh->type_block || !sh->lq_block || !sh->symbol_block || !sh->struct_member_block) {
      free(sh->str_block);
      free(sh->type_block);
      free(sh->lq_block);
      free(sh->symbol_block);
      free(sh->struct_member_block);
      shader_maps_term(&output);
      return false;
   }

   GLSL_MAP_FOREACH(e, output.string) {
      const char *str = e->k;
      int idx = (uintptr_t)e->v;
      strcpy(&sh->str_block[idx], str);
   }

   GLSL_MAP_FOREACH(e, output.lq) {
      const LayoutQualifier *lq = e->k;
      int idx = (uintptr_t)e->v;
      sh->lq_block[idx] = *lq;
   }

   GLSL_MAP_FOREACH(e, output.type) {
      const SymbolType *t = e->k;
      int idx = (uintptr_t)e->v;
      SymbolType *dest = & sh->type_block[idx];
      *dest = *t;
      dest->name = &sh->str_block[(uintptr_t)glsl_map_get(output.string, t->name)];
      switch(t->flavour) {
         case SYMBOL_PRIMITIVE_TYPE:
            break;   /* Take no action */
         case SYMBOL_ARRAY_TYPE:
            dest->u.array_type.member_type = &sh->type_block[(uintptr_t)glsl_map_get(output.type, t->u.array_type.member_type)];
            break;
         case SYMBOL_STRUCT_TYPE:
            dest->u.struct_type.member = &sh->struct_member_block[((int *)glsl_map_get(output.struct_members, t->u.struct_type.member))[1]];
            break;
         case SYMBOL_BLOCK_TYPE:
            dest->u.block_type.member = &sh->struct_member_block[((int *)glsl_map_get(output.struct_members, t->u.block_type.member))[1]];
            dest->u.block_type.lq = t->u.block_type.lq ? &sh->lq_block[(uintptr_t)glsl_map_get(output.lq, t->u.block_type.lq)] : NULL;
            dest->u.block_type.layout = NULL;
            break;
         default:
            unreachable();
            break;
      }
   }

   GLSL_MAP_FOREACH(e, output.symbol) {
      const Symbol *s = e->k;
      int idx = (uintptr_t)e->v;
      Symbol *dest = &sh->symbol_block[idx];
      *dest = *s;
      dest->name = &sh->str_block[(uintptr_t)glsl_map_get(output.string, s->name)];
      dest->type = &sh->type_block[(uintptr_t)glsl_map_get(output.type, s->type)];
      if (s->flavour == SYMBOL_VAR_INSTANCE) {
         dest->u.var_instance.compile_time_value = NULL;
         dest->u.var_instance.block_info.block_symbol = NULL;
         dest->u.var_instance.block_info.layout = NULL;
      } else {
         dest->u.interface_block.block_data_type = &sh->type_block[(uintptr_t)glsl_map_get(output.type, s->u.interface_block.block_data_type)];
      }

      glsl_map_put(symbol_map, s, dest);
   }

   GLSL_MAP_FOREACH(e, output.struct_members) {
      const StructMember *sm = e->k;
      int *v = e->v;
      for (int i=0; i<v[0]; i++) {
         int idx = v[1] + i;
         sh->struct_member_block[idx] = sm[i];
         sh->struct_member_block[idx].name = &sh->str_block[(uintptr_t)glsl_map_get(output.string, sm[i].name)];
         if (sm[i].layout != NULL)
            sh->struct_member_block[idx].layout = &sh->lq_block[(uintptr_t)glsl_map_get(output.lq, sm[i].layout)];
         sh->struct_member_block[idx].type = &sh->type_block[(uintptr_t)glsl_map_get(output.type, sm[i].type)];
      }
   }

   shader_maps_term(&output);

   return true;
}

static Dataflow *dprev_apply_opt_map(Dataflow *d, void *data) {
   Map *opt_map = data;

   for (int i=0; i<d->dependencies_count; i++) {
      if (d->d.dependencies[i] != NULL) {
         Dataflow *new = glsl_map_get(opt_map, d->d.dependencies[i]);
         if (new != NULL)
            d->d.dependencies[i] = new;
      }
   }

   return d;
}

static void array_apply_opt_map(Dataflow **arr, unsigned count, Map *opt_map) {
   glsl_dataflow_visit_array(arr, 0, count, opt_map, dprev_apply_opt_map, NULL);
   for (unsigned i=0; i<count; i++) {
      if (arr[i] != NULL) {
         Dataflow *new = glsl_map_get(opt_map, arr[i]);
         if (new != NULL) arr[i] = new;
      }
   }
}

struct replace_info {
   Map *replace_map;

   int *output_offset;
   Dataflow **new_outputs;

   const SSABlock *blocks;

   int id;
   Dataflow **guards_mem;
   Dataflow **guards_true;
   Dataflow **guards_false;
};

void glsl_predicate_dataflow(Dataflow *d, Dataflow *g) {
   if (g->flavour != DATAFLOW_CONST || !g->u.constant.value) {
      if (glsl_dataflow_affects_memory(d->flavour)) {
         if (d->d.addr_store.cond == NULL) d->d.addr_store.cond = g;
         else d->d.addr_store.cond = glsl_dataflow_construct_binary_op(DATAFLOW_LOGICAL_AND, d->d.addr_store.cond, g);
      }

      switch (d->flavour) {
      case DATAFLOW_SG_ELECT:
         d->d.unary_op.operand = glsl_dataflow_construct_binary_op(DATAFLOW_LOGICAL_AND, d->d.unary_op.operand, g);
         break;
      default: /* Do nothing */
         break;
      }
   }
}

static void replace_externals(Dataflow *d, void *data) {
   struct replace_info *info = data;

   d->age += info->id * 1000;

   glsl_predicate_dataflow(d, info->guards_mem[info->id]);

   if (d->flavour != DATAFLOW_EXTERNAL) return;

   glsl_map_put(info->replace_map, d, info->new_outputs[info->output_offset[d->u.external.block] + d->u.external.output]);
}

static bool is_opable_type(DataflowType t) {
   return t == DF_FLOAT || t == DF_INT || t == DF_UINT || t == DF_BOOL;
}

static Dataflow *condition_phi(Dataflow *phi, struct replace_info *info) {
   Dataflow **guards = info->blocks[phi->u.phi.in_a].next_true == info->id ? info->guards_true : info->guards_false;
   if (phi->u.phi.in_b != -1)
      return glsl_dataflow_construct_ternary_op(DATAFLOW_CONDITIONAL, guards[phi->u.phi.in_a], phi->d.binary_op.left,
                                                                                               phi->d.binary_op.right);
   else
      return glsl_dataflow_construct_ternary_op(DATAFLOW_CONDITIONAL, guards[phi->u.phi.in_a], phi->d.binary_op.left,
                                                                                               condition_phi(phi->d.binary_op.right, info));
}

static Dataflow *replace_phis(Dataflow *d, void *data) {
   if (d->flavour != DATAFLOW_PHI) return d;

   struct replace_info *info = data;
   /* TODO: This is a hack. The dodgy way this gets set up means we have
    * impossible phi nodes. Ignore them here */
   if (!is_opable_type(d->type))
      glsl_map_put(info->replace_map, d, d->d.binary_op.left);
   else
      glsl_map_put(info->replace_map, d, condition_phi(d, info));

   return NULL;
}

static void get_reverse_postordering(SSABlock *blocks, int n_blocks, int i, int *ordering, int *order_pos, bool *seen) {
   if (seen[i]) return;
   seen[i] = true;

   if (blocks[i].next_true  != -1) get_reverse_postordering(blocks, n_blocks, blocks[i].next_true,  ordering, order_pos, seen);
   if (blocks[i].next_false != -1) get_reverse_postordering(blocks, n_blocks, blocks[i].next_false, ordering, order_pos, seen);
   ordering[(*order_pos)--] = i;
}

static Dataflow *block_pred_from_cd(const struct cd_set *cds, const SSABlock *blocks, int id, int dom,
                                    const int *output_offset, Dataflow **outputs)
{
   const struct cd_set *cd = &cds[id];

   if (cd->n == 0 || id == dom) return glsl_dataflow_construct_const_bool(true);
   else {
      Dataflow *cond = glsl_dataflow_construct_const_bool(false);
      for (int i=0; i<cd->n; i++) {
         int pred_node = cd->dep[i].node;
         Dataflow *c = outputs[output_offset[pred_node] + blocks[pred_node].successor_condition];
         if (!cd->dep[i].cond) c = glsl_dataflow_construct_unary_op(DATAFLOW_LOGICAL_NOT, c);
         c = glsl_dataflow_construct_binary_op(DATAFLOW_LOGICAL_AND, c, block_pred_from_cd(cds, blocks, pred_node, dom, output_offset, outputs));
         cond = glsl_dataflow_construct_binary_op(DATAFLOW_LOGICAL_OR, cond, c);
      }
      return cond;
   }
}

static void flatten_and_predicate(SSAShader *sh) {
   SSABlock *blocks = sh->blocks;
   int n_blocks = sh->n_blocks;

   int *output_offset = glsl_safemem_malloc(n_blocks * sizeof(int));

   int *reverse_postordering = glsl_safemem_malloc(n_blocks * sizeof(int));
   bool *seen = glsl_safemem_malloc(n_blocks * sizeof(bool));
   for (int i=0; i<n_blocks; i++) seen[i] = false;
   int order_pos = n_blocks - 1;
   get_reverse_postordering(blocks, n_blocks, 0, reverse_postordering, &order_pos, seen);
   assert(order_pos == -1);
   glsl_safemem_free(seen);

   output_offset[0] = 0;
   for (int i=1; i<n_blocks; i++) output_offset[i] = output_offset[i-1] + blocks[i-1].n_outputs;

   SSABlock *new_block = glsl_safemem_malloc(sizeof(SSABlock));
   new_block->id = 0;
   new_block->n_outputs = 0;
   for (int i=0; i<n_blocks; i++) new_block->n_outputs += blocks[i].n_outputs;
   new_block->outputs = glsl_safemem_malloc(new_block->n_outputs * sizeof(Dataflow *));
   new_block->successor_condition = -1;
   new_block->next_true = -1;
   new_block->next_false = -1;
   new_block->barrier = false;

   for (int i=0; i<n_blocks; i++) {
      for (int j=0; j<blocks[i].n_outputs; j++) {
         new_block->outputs[ output_offset[i] + j ] = blocks[i].outputs[j];
      }
   }

   for (int i=0; i<sh->n_outputs; i++) {
      if (sh->outputs[i].block != -1) {
         sh->outputs[i].output += output_offset[sh->outputs[i].block];
         sh->outputs[i].block = 0;
      }
   }

   struct abstract_cfg *cfg = glsl_alloc_abstract_cfg_ssa(blocks, n_blocks);
   int *idom = glsl_alloc_idoms(cfg);
   struct cd_set *cd = glsl_cd_sets_alloc(cfg);
   glsl_free_abstract_cfg(cfg);

   Dataflow **guards_mem   = glsl_safemem_malloc(n_blocks * sizeof(Dataflow *));
   Dataflow **guards_true  = glsl_safemem_malloc(n_blocks * sizeof(Dataflow *));
   Dataflow **guards_false = glsl_safemem_malloc(n_blocks * sizeof(Dataflow *));

   for (int i=0; i<n_blocks; i++) {
      int id = reverse_postordering[i];

      glsl_dataflow_set_age(id * 1000);

      guards_mem[id]   = block_pred_from_cd(cd, blocks, id, 0,                           output_offset, new_block->outputs);
      guards_false[id] = block_pred_from_cd(cd, blocks, id, idom[blocks[id].next_false], output_offset, new_block->outputs);
      if (blocks[id].successor_condition != -1)
         guards_true[id] = block_pred_from_cd(cd, blocks, id, idom[blocks[id].next_true], output_offset, new_block->outputs);

      struct replace_info info = { .replace_map = glsl_map_new(), .output_offset = output_offset, .new_outputs = new_block->outputs, .blocks = blocks,
                                   .id = id, .guards_mem = guards_mem, .guards_true = guards_true, .guards_false = guards_false };
      glsl_dataflow_visit_array(new_block->outputs, output_offset[id], output_offset[id] + blocks[id].n_outputs, &info, NULL, replace_externals);
      glsl_dataflow_visit_array(new_block->outputs, output_offset[id], output_offset[id] + blocks[id].n_outputs, info.replace_map, dprev_apply_opt_map, NULL);
      for (int i=output_offset[id]; i<output_offset[id] + blocks[id].n_outputs; i++) {
         Dataflow *new = glsl_map_get(info.replace_map, new_block->outputs[i]);
         if (new != NULL)
            new_block->outputs[i] = new;
      }
      glsl_map_delete(info.replace_map);
      info.replace_map = glsl_map_new();
      glsl_dataflow_visit_array(new_block->outputs, output_offset[id], output_offset[id] + blocks[id].n_outputs, &info, replace_phis, NULL);
      glsl_dataflow_visit_array(new_block->outputs, output_offset[id], output_offset[id] + blocks[id].n_outputs, info.replace_map, dprev_apply_opt_map, NULL);
      for (int i=output_offset[id]; i<output_offset[id] + blocks[id].n_outputs; i++) {
         Dataflow *new = glsl_map_get(info.replace_map, new_block->outputs[i]);
         if (new != NULL)
            new_block->outputs[i] = new;
      }
      glsl_map_delete(info.replace_map);

      if (blocks[id].successor_condition != -1) {
         guards_true[id] = glsl_dataflow_construct_binary_op(DATAFLOW_LOGICAL_AND, guards_true[id], new_block->outputs[output_offset[id]+blocks[id].successor_condition]);
         guards_false[id] = glsl_dataflow_construct_binary_op(DATAFLOW_LOGICAL_AND, guards_false[id], glsl_dataflow_construct_unary_op(DATAFLOW_LOGICAL_NOT, new_block->outputs[output_offset[id]+blocks[id].successor_condition]));
      }
   }

   glsl_cd_sets_free(cd, n_blocks);
   glsl_safemem_free(idom);

   glsl_safemem_free(reverse_postordering);
   glsl_safemem_free(output_offset);
   glsl_safemem_free(guards_mem);
   glsl_safemem_free(guards_true);
   glsl_safemem_free(guards_false);

   for(int i=0; i<n_blocks; i++) glsl_safemem_free(blocks[i].outputs);
   glsl_safemem_free(blocks);

   sh->n_blocks = 1;
   sh->blocks = new_block;
}

/* Currently all stores have to happen in the last block. Exposing this to the
 * compiler during optimisation allows us to make good decisions, like only
 * allocating constants to store during the last block instead of at the point
 * they are stored */
static void sink_stores(SSAShader *sh) {
   unsigned c = 0;
   for (int i=0; i<sh->n_outputs; i++) {
      if (sh->outputs[i].block != -1 && sh->outputs[i].block != sh->n_blocks-1)
         c++;
   }

   if (c == 0) return;

   SSABlock *b = &sh->blocks[sh->n_blocks-1];
   unsigned off = b->n_outputs;

   b->outputs = glsl_safemem_realloc(b->outputs, (b->n_outputs + c) * sizeof(Dataflow *));
   b->n_outputs += c;

   for (int i=0; i < sh->n_outputs; i++) {
      if (sh->outputs[i].block == -1 || sh->outputs[i].block == sh->n_blocks-1)
         continue;

      Dataflow *orig = sh->blocks[sh->outputs[i].block].outputs[sh->outputs[i].output];
      b->outputs[off] = glsl_dataflow_construct_external(orig->type, sh->outputs[i].block, sh->outputs[i].output);
      sh->outputs[i].block  = sh->n_blocks - 1;
      sh->outputs[i].output = off++;
   }
}

struct opt_data {
   SSABlock *blocks;
   Map      *map;
};

static void dpostv_find_phid_constants(Dataflow *d, void *data) {
   struct opt_data *dat = data;
   Map *unphi = dat->map;
   SSABlock *blocks = dat->blocks;

   if (d->flavour != DATAFLOW_PHI) return;

   Dataflow *left  = d->d.binary_op.left;
   Dataflow *right = d->d.binary_op.right;
   if (left->flavour  != DATAFLOW_EXTERNAL) return;
   if (right->flavour != DATAFLOW_EXTERNAL) return;

   Dataflow *out_left  = blocks[left->u.external.block ].outputs[left->u.external.output];
   Dataflow *out_right = blocks[right->u.external.block].outputs[right->u.external.output];

   if (out_left->flavour == DATAFLOW_CONST && out_right->flavour == DATAFLOW_CONST &&
       out_left->u.constant.value == out_right->u.constant.value)
   {
      /* Since they are the same constant, choose one arbitrarily */
      glsl_map_put(unphi, d, left);
   }

   if (out_left->flavour == DATAFLOW_EXTERNAL &&
       out_left->u.external.block  == right->u.external.block &&
       out_left->u.external.output == right->u.external.output)
   {
      glsl_map_put(unphi, d, right);
   }
   if (out_right->flavour == DATAFLOW_EXTERNAL &&
       out_right->u.external.block  == left->u.external.block &&
       out_right->u.external.output == left->u.external.output)
   {
      glsl_map_put(unphi, d, left);
   }
}

static void unphi_constants(SSABlock *blocks, int n_blocks) {
   for (int i=0; i<n_blocks; i++) {
      SSABlock *b = &blocks[i];

      struct opt_data data;
      data.blocks = blocks;
      data.map    = glsl_map_new();
      glsl_dataflow_visit_array(b->outputs, 0, b->n_outputs, &data, NULL, dpostv_find_phid_constants);

      array_apply_opt_map(b->outputs, b->n_outputs, data.map);

      glsl_map_delete(data.map);
   }
}

static Dataflow *dprev_promote_constants(Dataflow *d, void *data) {
   struct opt_data *dat = data;
   /* Don't look at phi node's children. They can't be promoted */
   if (d->flavour == DATAFLOW_PHI)      return NULL;
   if (d->flavour != DATAFLOW_EXTERNAL) return d;

   SSABlock *b   = &dat->blocks[d->u.external.block];
   Dataflow *out = b->outputs[d->u.external.output];
   if (out->flavour == DATAFLOW_CONST) {
      Dataflow *promoted = glsl_dataflow_construct_const_value(out->type, out->u.constant.value);
      glsl_map_put(dat->map, d, promoted);
   }
   if (out->flavour == DATAFLOW_UNIFORM) {
      Dataflow *promoted = glsl_dataflow_construct_buffer(DATAFLOW_UNIFORM, out->type, out->u.buffer.index, out->u.buffer.offset);
      glsl_map_put(dat->map, d, promoted);
   }
   if (out->flavour == DATAFLOW_EXTERNAL) {
      Dataflow *promoted = glsl_dataflow_construct_external(out->type, out->u.external.block, out->u.external.output);
      glsl_map_put(dat->map, d, promoted);
   }

   if (out->flavour == DATAFLOW_GET_VEC4_COMPONENT) {
      if (out->d.unary_op.operand->flavour == DATAFLOW_VECTOR_LOAD) {
         Dataflow *a = out->d.unary_op.operand->d.unary_op.operand;
         if (a->flavour == DATAFLOW_ADDRESS) {
            Dataflow *u = a->d.unary_op.operand;
            if (u->flavour == DATAFLOW_UNIFORM_BUFFER) {
               Dataflow *buf_copy = glsl_dataflow_construct_buffer(DATAFLOW_UNIFORM_BUFFER, u->type, u->u.buffer.index, u->u.buffer.offset);
               Dataflow *addr_copy = glsl_dataflow_construct_address(buf_copy);
               Dataflow *load_copy = glsl_dataflow_construct_vector_load(addr_copy);
               Dataflow *promoted = glsl_dataflow_construct_get_vec4_component(out->u.get_vec4_component.component_index, load_copy, out->type);
               glsl_map_put(dat->map, d, promoted);
            }
         }
      }
   }

   /* This has no children, so could return d or NULL here */
   return NULL;
}

static void promote_constants(SSABlock *blocks, int n_blocks) {
   for (int i=0; i<n_blocks; i++) {
      SSABlock *b = &blocks[i];

      struct opt_data data;
      data.blocks = blocks;
      data.map = glsl_map_new();
      glsl_dataflow_visit_array(b->outputs, 0, b->n_outputs, &data, dprev_promote_constants, NULL);

      array_apply_opt_map(b->outputs, b->n_outputs, data.map);

      glsl_map_delete(data.map);
   }
}

void dpostv_simplify_recursive(Dataflow *d, void *data) {
   Map *opt_map = data;

   for (int i=0; i<d->dependencies_count; i++) {
      if (d->d.dependencies[i] != NULL) {
         d->d.dependencies[i] = glsl_map_get(opt_map, d->d.dependencies[i]);
      }
   }

   Dataflow *simplified = glsl_dataflow_simplify(d);
   if (simplified == NULL) simplified = d;

   glsl_map_put(opt_map, d, simplified);
}

void simplify_dataflow(SSABlock *blocks, int n_blocks) {
   for (int i=0; i<n_blocks; i++) {
      SSABlock *b = &blocks[i];

      Map *opt_map = glsl_map_new();
      glsl_dataflow_visit_array(b->outputs, 0, b->n_outputs, opt_map, NULL, dpostv_simplify_recursive);

      /* Opt map is applied as we go along this time because it is postv */
      for (int i=0; i<b->n_outputs; i++) {
         Dataflow *new = glsl_map_get(opt_map, b->outputs[i]);
         if (new != NULL)
            b->outputs[i] = new;
      }

      glsl_map_delete(opt_map);
   }
}

void dpostv_find_externals(Dataflow *df, void *data) {
   if (df->flavour == DATAFLOW_EXTERNAL) glsl_dataflow_chain_append(data, df);
}

static void block_find_active_externals(SSABlock *b, DataflowVisitor *dfv, bool *is_output, DataflowChain *externals) {
   if (b->successor_condition != -1) is_output[b->successor_condition] = true;

   for (int i=0; i<b->n_outputs; i++) {
      if (is_output[i]) glsl_dataflow_visitor_accept(dfv, b->outputs[i], externals, NULL, dpostv_find_externals);
   }
}

static void single_block_activity(SSABlock *b, DataflowVisitor *dfv, bool **is_output, DataflowChain *externals, SSABlock *blocks, int n_blocks)
{
   bool *next_blocks = glsl_safemem_calloc(n_blocks, sizeof(bool));

   /* Ensure that all memory stores are marked as active */
   for (int i=0; i<b->n_outputs; i++) {
      if (glsl_dataflow_affects_memory(b->outputs[i]->flavour)) is_output[b->id][i] = true;
   }

   block_find_active_externals(b, dfv, is_output[b->id], &externals[b->id]);

   /* Transfer output status to nodes that are loaded */
   for (DataflowChainNode *n = externals[b->id].head; n; n=n->l.next) {
      const Dataflow *d = n->ptr;
      assert(d->flavour == DATAFLOW_EXTERNAL);

      /* If this is already an output, don't re-add the source block */
      if (is_output[d->u.external.block][d->u.external.output]) continue;

      is_output[d->u.external.block][d->u.external.output] = true;
      next_blocks[d->u.external.block] = true;
   }

   /* Visit the blocks that were identified as needing visiting */
   for (int i=0; i<n_blocks; i++) {
      if (next_blocks[i])
         single_block_activity(&blocks[i], dfv, is_output, externals, blocks, n_blocks);
   }

   glsl_safemem_free(next_blocks);
}

static void eliminate_dead_code(SSAShader *sh)
{
   DataflowVisitor dfv;

   /* Allocate an array to store which outputs are live */
   bool **is_output         = malloc_fast(sh->n_blocks * sizeof(bool *));
   DataflowChain *externals = malloc_fast(sh->n_blocks * sizeof(DataflowChain));
   for (int i=0; i<sh->n_blocks; i++) {
      is_output[i] = malloc_fast(sh->blocks[i].n_outputs * sizeof(bool));
      memset(is_output[i], 0, sh->blocks[i].n_outputs * sizeof(bool));

      glsl_dataflow_chain_init(&externals[i]);
   }

   /* Mark as active any block outputs that are shader outputs */
   for (int i=0; i<sh->n_outputs; i++) {
      if (sh->outputs[i].block != -1)
         is_output[sh->outputs[i].block][sh->outputs[i].output] = true;
   }

   /* Pass over the IR determining which outputs are live. */
   glsl_dataflow_visitor_begin(&dfv);
   for (int i=0; i<sh->n_blocks; i++)
      single_block_activity(&sh->blocks[i], &dfv, is_output, externals, sh->blocks, sh->n_blocks);
   glsl_dataflow_visitor_end(&dfv);

   /* Actually eliminate the dead code, now that we know what it is */
   int **output_mapping = malloc_fast(sh->n_blocks * sizeof(int *));
   for (int i=0; i<sh->n_blocks; i++) {
      SSABlock *b = &sh->blocks[i];

      output_mapping[i] = malloc_fast(b->n_outputs * sizeof(int));
      int out_count = 0;
      for (int j=0; j<b->n_outputs; j++) {
         if (is_output[i][j]) output_mapping[i][j] = out_count++;
         else output_mapping[i][j] = -1;
      }
      Dataflow **new_outputs = glsl_safemem_malloc(out_count * sizeof(Dataflow *));
      for (int j=0; j<b->n_outputs; j++) {
         if (is_output[i][j]) new_outputs[output_mapping[i][j]] = b->outputs[j];
      }

      if (b->successor_condition != -1)
         b->successor_condition = output_mapping[i][b->successor_condition];

      glsl_safemem_free(b->outputs);
      b->n_outputs = out_count;
      b->outputs = new_outputs;
   }

   for (int i=0; i<sh->n_outputs; i++) {
      if (sh->outputs[i].block != -1)
         sh->outputs[i].output = output_mapping[sh->outputs[i].block][sh->outputs[i].output];
   }

   for (int i=0; i<sh->n_blocks; i++) {
      for (DataflowChainNode *n=externals[i].head; n; n=n->l.next) {
         Dataflow *d = n->ptr;
         d->u.external.output = output_mapping[d->u.external.block][d->u.external.output];
      }
   }
}

static bool copy_shader_ir(IRShader *ir, const SSAShader *sh)
{
   ir->num_outputs = sh->n_outputs;
   ir->outputs = malloc(ir->num_outputs * sizeof(IROutput));
   if (ir->num_outputs > 0 && ir->outputs == NULL) return false;

   memcpy(ir->outputs, sh->outputs, sh->n_outputs * sizeof(IROutput));

   ir->blocks = calloc(sh->n_blocks, sizeof(CFGBlock));
   if (!ir->blocks) return false;
   ir->num_cfg_blocks = sh->n_blocks;

   /* Now do the actual copying out */
   for (int i=0; i<sh->n_blocks; i++) {
      SSABlock *b = &sh->blocks[i];
      CFGBlock *b_out = &ir->blocks[i];

      if (!glsl_ir_copy_block(b_out, b->outputs, b->n_outputs))
         continue;

      b_out->successor_condition = b->successor_condition;
      b_out->next_if_true = b->next_true;
      b_out->next_if_false = b->next_false;
      b_out->barrier = b->barrier;
   }

   return true;
}

static void initialise_interface_symbols(BasicBlock *entry_block, Map *initial_values) {
   /* XXX Default scalar values are hateful but the algorithms assume all variables
    * are initialised. Come up with something better */
   BasicBlockList *l = glsl_basic_block_get_reverse_postorder_list(entry_block);
   for (BasicBlockList *b = l; b != NULL; b = b->next) {
      GLSL_MAP_FOREACH(e, b->v->scalar_values) {
         Dataflow **scalar_values = glsl_symbol_get_default_scalar_values(e->k);
         glsl_basic_block_set_scalar_values(entry_block, e->k, scalar_values);
      }
   }

   GLSL_MAP_FOREACH(e, initial_values) {
      glsl_basic_block_set_scalar_values(entry_block, e->k, e->v);
   }
}

static void generate_compute_variables(BasicBlock *entry_block, unsigned wg_size[3], Symbol *shared_block, bool multicore, bool clamp_shared_idx, uint32_t shared_mem_per_core)
{
   const Symbol *s_l_idx = glsl_stdlib_get_variable(GLSL_STDLIB_VAR__IN__UINT__GL_LOCALINVOCATIONINDEX);
   const Symbol *s_l_id  = glsl_stdlib_get_variable(GLSL_STDLIB_VAR__IN__UVEC3__GL_LOCALINVOCATIONID);
   const Symbol *s_wg_id = glsl_stdlib_get_variable(GLSL_STDLIB_VAR__IN__UVEC3__GL_WORKGROUPID);
   const Symbol *s_g_id  = glsl_stdlib_get_variable(GLSL_STDLIB_VAR__IN__UVEC3__GL_GLOBALINVOCATIONID);

   const Symbol *s_n_sgs = glsl_stdlib_get_variable(GLSL_STDLIB_VAR__IN__HIGHP__UINT____NUMSUBGROUPS);
   const Symbol *s_sg_id = glsl_stdlib_get_variable(GLSL_STDLIB_VAR__IN__HIGHP__UINT____SUBGROUPID);

   glsl_generate_compute_variables(s_l_idx, s_l_id, s_wg_id, s_g_id, s_n_sgs, s_sg_id,
                                   entry_block, wg_size, shared_block, multicore, clamp_shared_idx, shared_mem_per_core);
}

static void uint15_div_by_constant(Dataflow** div, Dataflow** rem, Dataflow* num, uint32_t divisor)
{
   assert(divisor != 0);

   if (divisor >= (1 << 15))
   {
      *div = glsl_dataflow_construct_const_uint(0);
      *rem = num;
   }
   else if (gfx_is_power_of_2(divisor))
   {
      *div = glsl_dataflow_construct_binary_op(DATAFLOW_SHR, num, glsl_dataflow_construct_const_uint(gfx_log2(divisor)));
      *rem = glsl_dataflow_construct_binary_op(DATAFLOW_BITWISE_AND, num, glsl_dataflow_construct_const_uint(divisor - 1));
   }
   else
   {
      uint32_t shift = 32u - 15u + gfx_msb(divisor);
      uint32_t mul = (1u << shift) / divisor + 1u;
      *div = glsl_dataflow_construct_binary_op(DATAFLOW_MUL24, num, glsl_dataflow_construct_const_uint(mul));
      *div = glsl_dataflow_construct_binary_op(DATAFLOW_SHR, *div, glsl_dataflow_construct_const_uint(shift));
      *rem = glsl_dataflow_construct_binary_op(DATAFLOW_MUL24, *div, glsl_dataflow_construct_const_uint(divisor));
      *rem = glsl_dataflow_construct_binary_op(DATAFLOW_SUB, num, *rem);
   }
}

void glsl_generate_compute_variables(const Symbol *s_l_idx, const Symbol *s_l_id,
                                     const Symbol *s_wg_id, const Symbol *s_g_id,
                                     const Symbol *s_n_sgs, const Symbol *s_sg_id,
                                     BasicBlock *entry_block, const unsigned *wg_size, const Symbol *shared_block,
                                     bool multicore, bool clamp_shared_idx, uint32_t shared_mem_per_core)
{
   unsigned const shared_block_size = shared_block->u.interface_block.block_data_type->u.block_type.layout->u.struct_layout.size;
   unsigned items_per_wg = wg_size[0]*wg_size[1]*wg_size[2];

#if V3D_USE_CSD
   Dataflow *id0 = glsl_dataflow_construct_nullary_op(DATAFLOW_COMP_GET_ID0);
   Dataflow *id1 = glsl_dataflow_construct_nullary_op(DATAFLOW_COMP_GET_ID1);

   Dataflow *wg_id[3];
   wg_id[0] = glsl_dataflow_construct_binary_op(DATAFLOW_BITWISE_AND, id0, glsl_dataflow_construct_const_uint(UINT16_MAX));
   wg_id[1] = glsl_dataflow_construct_binary_op(DATAFLOW_SHR,         id0, glsl_dataflow_construct_const_uint(16));
   wg_id[2] = glsl_dataflow_construct_binary_op(DATAFLOW_BITWISE_AND, id1, glsl_dataflow_construct_const_uint(UINT16_MAX));

   Dataflow *l_idx = glsl_dataflow_construct_const_uint(0);
   Dataflow *m_idx = glsl_dataflow_construct_binary_op(DATAFLOW_SHR, id1, glsl_dataflow_construct_const_uint(16));
   if (items_per_wg > 1)
   {
      unsigned bits_for_l_idx = gfx_msb(gfx_umax(items_per_wg, 64) - 1) + 1;
      l_idx = glsl_dataflow_construct_binary_op(DATAFLOW_SHR, id1, glsl_dataflow_construct_const_uint(32 - bits_for_l_idx));
      m_idx = glsl_dataflow_construct_binary_op(DATAFLOW_BITWISE_AND, m_idx, glsl_dataflow_construct_const_uint(gfx_mask(16 - bits_for_l_idx)));
   }
#else
   // Fetch wg_in_mem << s | local_index, wg_id_offset[3] from TLB.
   Dataflow *tlb[4];
   glsl_dataflow_construct_frag_get_col(tlb, DF_UINT, 0);

   Dataflow *wg_id[3];
   for (unsigned i = 0; i != 3; ++i)
   {
      // Fetch base work-group ID from varyings, add in offset from TLB.
      wg_id[i] = glsl_dataflow_construct_linkable_value(DATAFLOW_IN, DF_UINT, i);
      wg_id[i] = glsl_dataflow_construct_address(wg_id[i]);
      wg_id[i] = glsl_dataflow_construct_vec4(wg_id[i], NULL, NULL, NULL);
      wg_id[i] = glsl_dataflow_construct_address_load(DATAFLOW_IN_LOAD, DF_UINT, wg_id[i]);
      wg_id[i] = glsl_dataflow_construct_binary_op(DATAFLOW_ADD, tlb[i+1], wg_id[i]);
   }

   Dataflow *l_idx, *m_idx;
   if (shared_block_size > 0)
   {
      unsigned bits_for_l_idx = gfx_log2(gfx_next_power_of_2(items_per_wg));
      l_idx = glsl_dataflow_construct_binary_op(DATAFLOW_BITWISE_AND, tlb[0], glsl_dataflow_construct_const_uint(gfx_mask(bits_for_l_idx)));
      m_idx = glsl_dataflow_construct_binary_op(DATAFLOW_SHR, tlb[0], glsl_dataflow_construct_const_uint(bits_for_l_idx));
   }
   else
   {
      l_idx = tlb[0];
      m_idx = NULL;
   }
#endif

   // Compute local ID from local invocation index.
   Dataflow *zero = glsl_dataflow_construct_const_uint(0);
   Dataflow *rem = l_idx;
   Dataflow *l_id[3] = { zero, zero, zero };
   if (wg_size[2] > 1)
      uint15_div_by_constant(&l_id[2], &rem, rem, wg_size[0] * wg_size[1]);
   if (wg_size[1] > 1)
      uint15_div_by_constant(&l_id[1], &rem, rem, wg_size[0]);
   l_id[0] = rem;

   // Compute global ID from local and work-group IDs.
   Dataflow *g_id[3];
   for (unsigned i = 0; i != 3; ++i)
   {
      Dataflow *wg_sz = glsl_dataflow_construct_const_uint(wg_size[i]);
      g_id[i] = glsl_dataflow_construct_binary_op(DATAFLOW_MUL24, wg_id[i], wg_sz);
      g_id[i] = glsl_dataflow_construct_binary_op(DATAFLOW_ADD, g_id[i], l_id[i]);
   }

   Dataflow *n_sgs = glsl_dataflow_construct_const_uint((items_per_wg + 15) / 16);
   Dataflow *sg_id = glsl_dataflow_construct_binary_op(DATAFLOW_SHR, l_idx, glsl_dataflow_construct_const_uint(4));

   glsl_basic_block_set_scalar_values(entry_block, s_l_idx, &l_idx);
   glsl_basic_block_set_scalar_values(entry_block, s_g_id, g_id);
   glsl_basic_block_set_scalar_values(entry_block, s_l_id, l_id);
   glsl_basic_block_set_scalar_values(entry_block, s_wg_id, wg_id);

   glsl_basic_block_set_scalar_values(entry_block, s_n_sgs, &n_sgs);
   glsl_basic_block_set_scalar_values(entry_block, s_sg_id, &sg_id);

   Dataflow *shared_block_start = glsl_dataflow_construct_const_uint(0u);
   if (shared_block_size > 0)
   {
      assert(shared_block_size <= shared_mem_per_core);
      // shared_block_start = m_idx * shared_block_size
      if (clamp_shared_idx)
         m_idx = glsl_dataflow_construct_binary_op(DATAFLOW_MIN, m_idx, glsl_dataflow_construct_const_uint(shared_mem_per_core / shared_block_size - 1));
      shared_block_start = glsl_dataflow_construct_binary_op(DATAFLOW_MUL24, m_idx, glsl_dataflow_construct_const_uint(shared_block_size));
      shared_block_start = glsl_dataflow_construct_binary_op(DATAFLOW_ADD, shared_block_start, glsl_dataflow_construct_nullary_op(DATAFLOW_SHARED_PTR));

#if !V3D_HAS_L2T_LOCAL_MEM
      if (multicore) {
         Dataflow *core_offset = glsl_dataflow_construct_nullary_op(DATAFLOW_GET_THREAD_INDEX);
         core_offset = glsl_dataflow_construct_binary_op(DATAFLOW_SHR, core_offset, glsl_dataflow_construct_const_uint(6));
         core_offset = glsl_dataflow_construct_binary_op(DATAFLOW_MUL24, core_offset, glsl_dataflow_construct_const_uint(shared_mem_per_core));
         shared_block_start = glsl_dataflow_construct_binary_op(DATAFLOW_ADD, shared_block_start, core_offset);
      }
#endif
   }
   glsl_basic_block_set_scalar_values(entry_block, shared_block, &shared_block_start);
}

/* XXX: Hackily copy this here to build a shared block symbol */
Symbol *glsl_construct_shared_block(const SymbolList *members)
{
   SymbolType *resultType   = malloc_fast(sizeof(SymbolType));
   resultType->flavour      = SYMBOL_BLOCK_TYPE;
   resultType->name         = glsl_intern("$$shared_block_type", false);
   resultType->scalar_count = 0;    /* Added as we go along */
   resultType->u.block_type.lq = NULL;

   int count = 0;
   for (SymbolListNode *n = members->head; n; n=n->next) count++;

   StructMember *memb = malloc_fast(sizeof(StructMember) * count);

   int i;
   SymbolListNode *n;
   for (n=members->head, i=0; n; n=n->next, i++) {
      memb[i].name   = n->s->name;
      memb[i].type   = n->s->type;
      memb[i].layout = NULL;
      memb[i].prec   = n->s->u.var_instance.prec_qual;
      memb[i].memq   = MEMORY_NONE;

      resultType->scalar_count += n->s->type->scalar_count;
   }
   assert(i == count);

   resultType->u.block_type.member_count = count;
   resultType->u.block_type.member       = memb;

   resultType->u.block_type.layout = malloc_fast(sizeof(MemLayout));
   glsl_mem_calculate_block_layout(resultType->u.block_type.layout, resultType, /*for_tmu*/true);
   /* Add padding to the end of the block. This ensures that all the copies
    * of the block that are used for concurrent invocations are correctly aligned */
   resultType->u.block_type.layout->u.struct_layout.size = gfx_uround_up(resultType->u.block_type.layout->u.struct_layout.size,
                                                                         resultType->u.block_type.layout->base_alignment);

   Qualifiers q = { .invariant = false,
                    .lq = NULL,
                    .sq = STORAGE_SHARED,
                    .iq = INTERP_SMOOTH,
                    .aq = AUXILIARY_NONE,
                    .pq = PREC_HIGHP,
                    .mq = MEMORY_NONE };

   Symbol *shared_block = malloc_fast(sizeof(Symbol));
   glsl_symbol_construct_interface_block(shared_block, glsl_intern("$$shared_vars", false),
                                         &primitiveTypes[PRIM_UINT], resultType, &q);

   for (n=members->head, i=0; n; n=n->next, i++) {
      n->s->u.var_instance.block_info_valid = true;
      n->s->u.var_instance.block_info.block_symbol = shared_block;
      n->s->u.var_instance.block_info.layout = &shared_block->u.interface_block.block_data_type->u.block_type.layout->u.struct_layout.member_layouts[i];
      n->s->u.var_instance.block_info.field_no = i;
   }
   return shared_block;
}

static void init_compiler_common(void)
{
   // Initialize string intern facility
   glsl_init_intern(1024);

   glsl_dataflow_reset_count();
   glsl_prim_init();
   glsl_stdlib_init();

   glsl_compile_error_reset();
}

static void iface_data_fill_default(IFaceData *data, ShaderFlavour flavour) {
   memset(data, 0, sizeof(IFaceData));

   switch(flavour) {
      case SHADER_COMPUTE:
         for (int i=0; i<3; i++) data->cs.wg_size[i] = 1;
         break;
      case SHADER_TESS_EVALUATION:
         data->tes.mode    = V3D_CL_TESS_TYPE_INVALID;
         data->tes.spacing = V3D_CL_TESS_EDGE_SPACING_EQUAL;
         break;
      case SHADER_GEOMETRY:
         data->gs.n_invocations = 1;
         break;
      default:
         break;
   }
}

static void set_iface_unsigned(unsigned *loc, unsigned new, bool *declared) {
   if (*declared && *loc != new)
      glsl_compile_error(ERROR_CUSTOM, 15, -1, "Inconsistent interface declaration");
   *loc = new;
   *declared = true;
}

static void set_gs_in_type(IFaceData *data, enum gs_in_type type, bool *declared) {
   if (*declared && data->gs.in != type)
      glsl_compile_error(ERROR_CUSTOM, 15, -1, "Inconsistent geometry shader input declarations");
   data->gs.in = type;
   *declared = true;
}

static void set_gs_out_type(IFaceData *data, v3d_cl_geom_prim_type_t type, bool *declared) {
   if (*declared && data->gs.out != type)
      glsl_compile_error(ERROR_CUSTOM, 15, -1, "Inconsistent geometry shader output declarations");
   data->gs.out = type;
   *declared = true;
}

static void set_tess_spacing(IFaceData *data, v3d_cl_tess_edge_spacing_t spacing, bool *declared) {
   if (*declared && data->tes.spacing != spacing)
      glsl_compile_error(ERROR_CUSTOM, 15, -1, "Inconsistent tessellation spacing declarations");
   data->tes.spacing = spacing;
   *declared = true;
}

static void set_tess_winding(IFaceData *data, bool cw, bool *declared) {
   if (*declared && data->tes.cw != cw)
      glsl_compile_error(ERROR_CUSTOM, 15, -1, "Inconsistent tessellation winding mode declarations");
   data->tes.cw = cw;
   *declared = true;
}

static void set_tess_eval_mode(IFaceData *data, v3d_cl_tess_type_t mode) {
   if (data->tes.mode != V3D_CL_TESS_TYPE_INVALID && data->tes.mode != mode)
      glsl_compile_error(ERROR_CUSTOM, 15, -1, "Inconsistent tessellation mode declarations");
   data->tes.mode = mode;
}

static void iface_data_fill(IFaceData *data, const Statement *ast, ShaderFlavour flavour) {
   if (flavour == SHADER_VERTEX) return;     /* No IFaceData for vertex shaders */

   iface_data_fill_default(data, flavour);

   bool size_declared             = false;
   bool tess_vertices_declared    = false;
   bool tess_spacing_declared     = false;
   bool tess_winding_declared     = false;
   bool gs_n_invocations_declared = false;
   bool gs_in_type_declared       = false;
   bool gs_out_type_declared      = false;
   bool gs_max_vertices_declared  = false;
   for (StatementChainNode *n = ast->u.ast.decls->first; n; n=n->next) {
      if (n->statement->flavour != STATEMENT_QUALIFIER_DEFAULT) continue;

      StorageQualifier sq = STORAGE_NONE;
      for (QualListNode *qn = n->statement->u.qualifier_default.quals->head; qn; qn=qn->next) {
         if (qn->q->flavour == QUAL_STORAGE) {
            sq = qn->q->u.storage;
            break;
         }
      }

      for (QualListNode *qn = n->statement->u.qualifier_default.quals->head; qn; qn=qn->next) {
         if (qn->q->flavour != QUAL_LAYOUT) continue;

         for (LayoutIDList *idn = qn->q->u.layout; idn; idn=idn->next) {
            if (sq == STORAGE_IN) {
               switch(idn->l->id) {
                  case LQ_SIZE_X:                  data->cs.wg_size[0] = idn->l->argument; break;
                  case LQ_SIZE_Y:                  data->cs.wg_size[1] = idn->l->argument; break;
                  case LQ_SIZE_Z:                  data->cs.wg_size[2] = idn->l->argument; break;
                  case LQ_EARLY_FRAGMENT_TESTS:    data->fs.early_tests = true;            break;
                  case LQ_SPACING_EQUAL:           set_tess_spacing(data, V3D_CL_TESS_EDGE_SPACING_EQUAL,           &tess_spacing_declared); break;
                  case LQ_SPACING_FRACTIONAL_EVEN: set_tess_spacing(data, V3D_CL_TESS_EDGE_SPACING_FRACTIONAL_EVEN, &tess_spacing_declared); break;
                  case LQ_SPACING_FRACTIONAL_ODD:  set_tess_spacing(data, V3D_CL_TESS_EDGE_SPACING_FRACTIONAL_ODD,  &tess_spacing_declared); break;
                  case LQ_CW:                      set_tess_winding(data, true,  &tess_winding_declared); break;
                  case LQ_CCW:                     set_tess_winding(data, false, &tess_winding_declared); break;
                  case LQ_POINT_MODE:              data->tes.point_mode  = true;                          break;
                  case LQ_INVOCATIONS:
                     set_iface_unsigned(&data->gs.n_invocations, idn->l->argument, &gs_n_invocations_declared);
                     break;
                  case LQ_POINTS:                  set_gs_in_type(data, GS_IN_POINTS,    &gs_in_type_declared); break;
                  case LQ_LINES:                   set_gs_in_type(data, GS_IN_LINES,     &gs_in_type_declared); break;
                  case LQ_LINES_ADJACENCY:         set_gs_in_type(data, GS_IN_LINES_ADJ, &gs_in_type_declared); break;
                  case LQ_TRIANGLES_ADJACENCY:     set_gs_in_type(data, GS_IN_TRIS_ADJ,  &gs_in_type_declared); break;
                  case LQ_TRIANGLES:
                     if (flavour == SHADER_GEOMETRY) set_gs_in_type(data, GS_IN_TRIANGLES, &gs_in_type_declared);
                     else                            set_tess_eval_mode(data, V3D_CL_TESS_TYPE_TRIANGLE);
                     break;
                  case LQ_ISOLINES:                set_tess_eval_mode(data, V3D_CL_TESS_TYPE_ISOLINES); break;
                  case LQ_QUADS:                   set_tess_eval_mode(data, V3D_CL_TESS_TYPE_QUAD);     break;
                  default: /* Do nothing */ break;
               }

               if (idn->l->id == LQ_SIZE_X || idn->l->id == LQ_SIZE_Y || idn->l->id == LQ_SIZE_Z)
                  size_declared = true;
            } else if (sq == STORAGE_OUT) {
               switch (idn->l->id) {
                  case LQ_POINTS:         set_gs_out_type(data, V3D_CL_GEOM_PRIM_TYPE_POINTS,         &gs_out_type_declared); break;
                  case LQ_LINE_STRIP:     set_gs_out_type(data, V3D_CL_GEOM_PRIM_TYPE_LINE_STRIP,     &gs_out_type_declared); break;
                  case LQ_TRIANGLE_STRIP: set_gs_out_type(data, V3D_CL_GEOM_PRIM_TYPE_TRIANGLE_STRIP, &gs_out_type_declared); break;
                  case LQ_MAX_VERTICES:
                     set_iface_unsigned(&data->gs.max_vertices, idn->l->argument, &gs_max_vertices_declared); break;
                  case LQ_VERTICES:
                     set_iface_unsigned(&data->tcs.vertices, idn->l->argument, &tess_vertices_declared);      break;

                  case LQ_BLEND_SUPPORT_MULTIPLY:
                  case LQ_BLEND_SUPPORT_SCREEN:
                  case LQ_BLEND_SUPPORT_OVERLAY:
                  case LQ_BLEND_SUPPORT_DARKEN:
                  case LQ_BLEND_SUPPORT_LIGHTEN:
                  case LQ_BLEND_SUPPORT_COLORDODGE:
                  case LQ_BLEND_SUPPORT_COLORBURN:
                  case LQ_BLEND_SUPPORT_HARDLIGHT:
                  case LQ_BLEND_SUPPORT_SOFTLIGHT:
                  case LQ_BLEND_SUPPORT_DIFFERENCE:
                  case LQ_BLEND_SUPPORT_EXCLUSION:
                  case LQ_BLEND_SUPPORT_HSL_HUE:
                  case LQ_BLEND_SUPPORT_HSL_SATURATION:
                  case LQ_BLEND_SUPPORT_HSL_COLOR:
                  case LQ_BLEND_SUPPORT_HSL_LUMINOSITY:
                  case LQ_BLEND_SUPPORT_ALL_EQUATIONS: data->fs.abq |= glsl_lq_to_abq(idn->l->id); break;

                  default: /* Do nothing */ break;
               }
            }
         }
      }
   }
   if (flavour == SHADER_TESS_CONTROL) {
      if (!tess_vertices_declared)
         glsl_compile_error(ERROR_CUSTOM, 15, -1, "tessellation control shader requires output patch size declaration");
      if (data->tcs.vertices <= 0 || data->tcs.vertices > V3D_MAX_PATCH_VERTICES)
         glsl_compile_error(ERROR_CUSTOM, 15, -1, "'vertices' declaration must be in the range [1, %d]", V3D_MAX_PATCH_VERTICES);
   }
   if (flavour == SHADER_GEOMETRY) {
      if (!gs_in_type_declared)
         glsl_compile_error(ERROR_CUSTOM, 15, -1, "geometry shader requires input type declaration");
      if (data->gs.n_invocations < 1 || data->gs.n_invocations > V3D_MAX_GEOMETRY_INVOCATIONS)
         glsl_compile_error(ERROR_CUSTOM, 15, -1, "geometry shader must declare invocations in the range [1, %d]",
                                                  V3D_MAX_GEOMETRY_INVOCATIONS);
      if (!gs_out_type_declared)
         glsl_compile_error(ERROR_CUSTOM, 15, -1, "geometry shader requires output type declaration");
      if (!gs_max_vertices_declared || data->gs.max_vertices > GLXX_CONFIG_MAX_GEOMETRY_OUTPUT_VERTICES)
      {
         glsl_compile_error(ERROR_CUSTOM, 15, -1, "geometry shader must declare max_vertices in the range [0, %d]",
                                                  GLXX_CONFIG_MAX_GEOMETRY_OUTPUT_VERTICES);
      }
   }

   if (flavour == SHADER_COMPUTE && size_declared) {
      static const unsigned max_size[3] = { GLXX_CONFIG_MAX_COMPUTE_GROUP_SIZE_X,
                                            GLXX_CONFIG_MAX_COMPUTE_GROUP_SIZE_Y,
                                            GLXX_CONFIG_MAX_COMPUTE_GROUP_SIZE_Z };

      for (int i=0; i<3; i++) {
         if (data->cs.wg_size[i] == 0 || data->cs.wg_size[i] > max_size[i])
            glsl_compile_error(ERROR_CUSTOM, 15, -1, "local size %d should be in the range (0, %d]", data->cs.wg_size[i], max_size[i]);
      }

      unsigned total_invocations = data->cs.wg_size[0] * data->cs.wg_size[1] * data->cs.wg_size[2];
      if (total_invocations > GLXX_CONFIG_MAX_COMPUTE_WORK_GROUP_INVOCATIONS)
         glsl_compile_error(ERROR_CUSTOM, 15, -1, "Compute invocations %d exceeds maximum %d",
                                                  total_invocations,
                                                  GLXX_CONFIG_MAX_COMPUTE_WORK_GROUP_INVOCATIONS);

   }
   /* No error for not declaring a size -- that must be deferred until link time */
   if (flavour == SHADER_COMPUTE && !size_declared)
      for (int i=0; i<3; i++) data->cs.wg_size[i] = 0;
}

void validate_fshader_out_interface(const struct if_usage *out, const IFaceData *iface_data) {
   bool frag_color = false, frag_data = false, frag_depth = false;
   for (unsigned i = 0; i<out->n; i++) {
      if (out->v[i].used && strcmp(out->v[i].symbol->name, "gl_FragColor") == 0) frag_color = true;
      if (out->v[i].used && strcmp(out->v[i].symbol->name, "gl_FragData" ) == 0) frag_data  = true;
      if (out->v[i].used && strcmp(out->v[i].symbol->name, "gl_FragDepth") == 0) frag_depth = true;
   }

   if (frag_color && frag_data)
      glsl_compile_error(ERROR_CUSTOM, 5, -1, "Use of both gl_FragColor and gl_FragData");

   if (iface_data->fs.early_tests && frag_depth)
      glsl_compile_error(ERROR_CUSTOM, 5, -1, "Use of gl_FragDepth while early_fragment_tests specified");
}

void validate_fshader_in_interface(const struct if_usage *in, const IFaceData *iface_data) {
   bool last_depth = false, last_stencil = false;
   for (unsigned i = 0; i<in->n; i++) {
      if (in->v[i].used && strcmp(in->v[i].symbol->name, "gl_LastFragDepthBRCM"  ) == 0) last_depth   = true;
      if (in->v[i].used && strcmp(in->v[i].symbol->name, "gl_LastFragStencilBRCM") == 0) last_stencil = true;
   }
   if ((last_depth || last_stencil) && iface_data->fs.early_tests)
      glsl_compile_error(ERROR_CUSTOM, 5, -1, "Reading depth or stencil buffer while early_fragment_tests specified");
}

void validate_gshader_out_interface(const struct if_usage *out, const IFaceData *iface_data) {
   unsigned output_word_count = 0;
   for (unsigned i=0; i<out->n; i++)
      if (out->v[i].used && strcmp(out->v[i].symbol->name, "__brcm_stdlib_header_words"))
         output_word_count += out->v[i].symbol->type->scalar_count;

   output_word_count *= iface_data->gs.max_vertices;
   if (output_word_count > GLXX_CONFIG_MAX_GEOMETRY_TOTAL_OUTPUT_COMPONENTS)
      glsl_compile_error(ERROR_CUSTOM, 5, -1, "Found %d geometry shader output components, max %d",
                                              output_word_count, GLXX_CONFIG_MAX_GEOMETRY_TOTAL_OUTPUT_COMPONENTS);
}

static void ssa_shader_optimise(SSAShader *sh, bool flatten) {
   if (flatten)
      flatten_and_predicate(sh);

   if (sh->n_blocks > 1) {
      sink_stores(sh);
      unphi_constants(sh->blocks, sh->n_blocks);
      promote_constants(sh->blocks, sh->n_blocks);
   }
   simplify_dataflow(sh->blocks, sh->n_blocks);

   eliminate_dead_code(sh);

   glsl_dataflow_cse(sh->blocks, sh->n_blocks);
}

static void dpostv_find_loads(Dataflow *df, void *data) {
   if (df->flavour == DATAFLOW_IN_LOAD) glsl_dataflow_chain_append(data, df);
}

static void hoist_loads(SSAShader *sh) {
   /* TODO: Hoist input loads into the first block. This isn't required, but for now we're going to
    * do it because it matches the old system, works for the new system and is simple. */
   for (int i=1; i<sh->n_blocks; i++) {
      SSABlock *b = &sh->blocks[i];

      DataflowChain loads;
      glsl_dataflow_chain_init(&loads);
      glsl_dataflow_visit_array(b->outputs, 0, b->n_outputs, &loads, NULL, dpostv_find_loads);

      int n_new = 0;
      for (DataflowChainNode *n = loads.head; n; n=n->l.next) n_new++;

      Dataflow **new_outs = glsl_safemem_malloc((sh->blocks[0].n_outputs + n_new) * sizeof(Dataflow *));
      memcpy(new_outs, sh->blocks[0].outputs, sh->blocks[0].n_outputs * sizeof(Dataflow *));

      Map *opt_map = glsl_map_new();
      int idx = sh->blocks[0].n_outputs;
      for (DataflowChainNode *n = loads.head; n; n=n->l.next, idx++) {
         /* Copy each load into the new_outs array */
         Dataflow *cur_addr = n->ptr->d.dependencies[0]->d.dependencies[0];
         Dataflow *cur_offset = NULL;
         assert(cur_addr->flavour == DATAFLOW_ADDRESS || cur_addr->flavour == DATAFLOW_ADD);
         if (cur_addr->flavour == DATAFLOW_ADD) {
            cur_offset = cur_addr->d.binary_op.right;
            cur_addr   = cur_addr->d.binary_op.left;
         }

         Dataflow *new_addr;
         assert(cur_addr->flavour == DATAFLOW_ADDRESS);
         assert(cur_addr->d.dependencies[0]->flavour == DATAFLOW_IN);
         Dataflow *new_in = glsl_dataflow_construct_linkable_value(DATAFLOW_IN, cur_addr->type, cur_addr->d.dependencies[0]->u.linkable_value.row);
         new_addr = glsl_dataflow_construct_address(new_in);

         if (cur_offset != NULL) {
            assert(cur_offset->flavour == DATAFLOW_CONST);
            Dataflow *const_offset = glsl_dataflow_construct_const_value(DF_UINT,  cur_offset->u.constant.value);
            new_addr = glsl_dataflow_construct_binary_op(DATAFLOW_ADD, new_addr, const_offset);
         }

         new_outs[idx] = glsl_dataflow_construct_address_load(DATAFLOW_IN_LOAD, n->ptr->type,
                                       glsl_dataflow_construct_vec4(new_addr, NULL, NULL, NULL));

         /* Create an external for the new output and add it to a map from the original */
         Dataflow *external = glsl_dataflow_construct_external(n->ptr->type, 0, idx);
         glsl_map_put(opt_map, n->ptr, external);
      }

      /* Switch in the new outputs to the first block */
      glsl_safemem_free(sh->blocks[0].outputs);

      sh->blocks[0].n_outputs += n_new;
      sh->blocks[0].outputs = new_outs;

      /* Apply the opt map to drop the loads from the original block */
      array_apply_opt_map(b->outputs, b->n_outputs, opt_map);
      glsl_map_delete(opt_map);
   }
}

static bool version_valid_for_flavour(ShaderFlavour flavour, int version) {
   switch (flavour) {
      case SHADER_VERTEX:
      case SHADER_FRAGMENT: return true;
      case SHADER_COMPUTE:  return version >= GLSL_SHADER_VERSION(3, 10, 1);
      case SHADER_GEOMETRY:
         return version >= GLSL_SHADER_VERSION(3, 20, 1) || glsl_ext_status(GLSL_EXT_GEOMETRY) != GLSL_DISABLED;
      case SHADER_TESS_CONTROL:
      case SHADER_TESS_EVALUATION:
         return version >= GLSL_SHADER_VERSION(3, 20, 1) || glsl_ext_status(GLSL_EXT_TESSELLATION) != GLSL_DISABLED;
      default: unreachable(); return false;
   }
}

static bool loop_or_barrier(BasicBlockList *l, Map *seen) {
   for ( ; l ; l=l->next) {
      glsl_map_put(seen, l->v, l->v);
      if (l->v->branch_target      && glsl_map_get(seen, l->v->branch_target))      return true;
      if (l->v->fallthrough_target && glsl_map_get(seen, l->v->fallthrough_target)) return true;

      if (l->v->barrier) return true;
   }
   return false;
}

static bool ssa_flattening_supported(BasicBlock *entry) {
   BasicBlockList *l = glsl_basic_block_get_reverse_postorder_list(entry);
   Map *seen = glsl_map_new();
   bool ret = !loop_or_barrier(l, seen);
   glsl_map_delete(seen);
   return ret;
}

static bool use_early_fragment_tests(const IRShader *s, bool requested) {
   bool visible_effects     = false;
   bool reads_depth_stencil = false;
   bool reads_sample_mask   = false;
   for (int i=0; i<s->num_cfg_blocks; i++) {
      const CFGBlock *b = &s->blocks[i];
      for (int j=0; j<b->num_dataflow; j++) {
         if (glsl_dataflow_affects_memory(b->dataflow[j].flavour)) visible_effects = true;
         if (b->dataflow[j].flavour == DATAFLOW_SAMPLE_MASK) reads_sample_mask = true;
         if (b->dataflow[j].flavour == DATAFLOW_FRAG_GET_DEPTH || b->dataflow[j].flavour == DATAFLOW_FRAG_GET_STENCIL)
            reads_depth_stencil = true;
      }
   }

   return (requested || !(visible_effects || reads_depth_stencil || reads_sample_mask));
}

#ifdef ANDROID
__attribute__((optimize("-O0")))
#endif
CompiledShader *glsl_compile_shader(ShaderFlavour flavour, const GLSL_SHADER_SOURCE_T *src, bool multicore, bool clamp_shared_idx, uint32_t shared_mem_per_core)
{
#ifdef KHRN_SHADER_DUMP_SOURCE
   glsl_shader_source_dump(src);
#endif

   glsl_fastmem_init();
   glsl_dataflow_begin_construction();
   init_compiler_common();

   // Set long jump point in case of error.
   if (setjmp(g_ErrorHandlerEnv) != 0)
   {
      /* Jumping back from an error. These functions must be safe to be called
       * even if the thing is not started or already shut down because we don't
       * know when error may occur. */
      glsl_term_lexer();
      glsl_stack_cleanup();
      glsl_fastmem_term();
      glsl_dataflow_end_construction();
      glsl_safemem_cleanup();
      return NULL;
   }

   // Call the parser to generate an AST from the source strings
   int version;
   if (!glsl_find_version(src->sourcec, src->sourcev, &version))
      glsl_compile_error(ERROR_PREPROCESSOR, 5, -1, "Invalid or unsupported version");

   Statement *ast = glsl_parse_ast(flavour, version, src->sourcec, src->sourcev);

   /* Check validity. May depend on extension enables, so must wait until after parsing */
   if (!version_valid_for_flavour(flavour, version))
      glsl_compile_error(ERROR_LINKER, 8, -1, "Shader stage not supported in language version");

   ShaderInterfaces *interfaces = glsl_shader_interfaces_new();

#if !V3D_USE_CSD
   if (flavour == SHADER_COMPUTE) {
      SymbolType *type = &primitiveTypes[PRIM_UVEC3];
      Qualifiers quals = { .invariant = false,
                           .lq = NULL,
                           .sq = STORAGE_IN,
                           .iq = INTERP_SMOOTH,
                           .aq = AUXILIARY_NONE,
                           .pq = PREC_MEDIUMP,
                           .mq = MEMORY_NONE };
      Symbol *compute_vary = malloc_fast(sizeof(Symbol));
      glsl_symbol_construct_var_instance(compute_vary, glsl_intern("$$comp_vary", false), type, &quals, NULL, NULL);
      glsl_shader_interfaces_update(interfaces, compute_vary);
   }
#endif

   uint64_t active_mask = glsl_stdlib_get_property_mask(flavour, version) | glsl_ext_get_symbol_mask();
   glsl_stdlib_interfaces_update(interfaces, active_mask);

   /* Walk the AST looking for interface variable symbols. They must be at global scope */
   for (StatementChainNode *n = ast->u.ast.decls->first; n; n=n->next) {
      if (n->statement->flavour == STATEMENT_DECL_LIST) {
         for (StatementChainNode *n2 = n->statement->u.decl_list.decls->first; n2; n2=n2->next)
            glsl_shader_interfaces_update(interfaces, n2->statement->u.var_decl.var);
      } else if (n->statement->flavour == STATEMENT_VAR_DECL)
         glsl_shader_interfaces_update(interfaces, n->statement->u.var_decl.var);
   }

   Symbol *shared_block = glsl_construct_shared_block(interfaces->shared);

   Map *symbol_ids = glsl_shader_interfaces_id_map(interfaces);

   IFaceData iface_data;
   iface_data_fill(&iface_data, ast, flavour);

   /* Normalise the AST */
   NStmtList *nast = glsl_nast_build(ast);

   /* Build basic block graph */
   BasicBlock *shader_start_block = glsl_basic_block_build(nast);

   /* Add interface definitions to a basic block prior to the shader */
   BasicBlock *entry_block = glsl_basic_block_construct();
   entry_block->fallthrough_target = shader_start_block;

   Map *initial_values = glsl_shader_interfaces_create_dataflow(interfaces, symbol_ids);
   initialise_interface_symbols(entry_block, initial_values);
   glsl_map_delete(initial_values);
   if (flavour == SHADER_COMPUTE)
      generate_compute_variables(entry_block, iface_data.cs.wg_size, shared_block, multicore, clamp_shared_idx, shared_mem_per_core);

   struct sh_usage symbs;
   if_symbols_from_symbol_list(&symbs.in,      interfaces->ins,      ast);
   if_symbols_from_symbol_list(&symbs.out,     interfaces->outs,     ast);
   if_symbols_from_symbol_list(&symbs.uniform, interfaces->uniforms, ast);
   if_symbols_from_symbol_list(&symbs.buffer,  interfaces->buffers,  ast);

   if (flavour == SHADER_FRAGMENT) {
      validate_fshader_out_interface(&symbs.out, &iface_data);
      validate_fshader_in_interface(&symbs.in, &iface_data);
   }

   if (flavour == SHADER_GEOMETRY)
      validate_gshader_out_interface(&symbs.out, &iface_data);

   if (flavour == SHADER_COMPUTE)
      iface_data.cs.shared_block_size = shared_block->u.interface_block.block_data_type->u.block_type.layout->u.struct_layout.size;

   CompiledShader *ret = glsl_compile_common(flavour, version, entry_block, &iface_data, &symbs, symbol_ids,
                                             /*unroll=*/true, /*activity_supported=*/true);

   /* No errors should be generated here because the error handler is no longer valid */

   glsl_safemem_free(symbs.in.v);
   glsl_safemem_free(symbs.out.v);
   glsl_safemem_free(symbs.uniform.v);
   glsl_safemem_free(symbs.buffer.v);

   glsl_map_delete(symbol_ids);

   glsl_dataflow_end_construction();
   glsl_fastmem_term();
   if (ret == NULL)
      glsl_safemem_cleanup();
#ifndef NDEBUG
   glsl_safemem_verify();
#endif

   return ret;
}

CompiledShader *glsl_compile_common(ShaderFlavour flavour, int version, BasicBlock *entry_block,
                                    const IFaceData *iface_data, const struct sh_usage *symbs,
                                    Map *symbol_ids, bool loop_unroll, bool activity_supported)
{
   CompiledShader *ret = NULL;

   if (setjmp(g_ErrorHandlerEnv) != 0) {
      /* Jumping back from an error. Only free the shader and signal an error.
       * The caller already needs to clean up memory, so leaving safemem is OK */
      glsl_compiled_shader_free(ret);
      return NULL;
   }

   glsl_basic_block_elim_dead(entry_block);

   if (flavour == SHADER_COMPUTE) {
      if (!glsl_wg_size_requires_barriers(iface_data->cs.wg_size)) {
         BasicBlockList *l = glsl_basic_block_get_reverse_postorder_list(entry_block);
         for (BasicBlockList *n = l; n != NULL; n=n->next)
            n->v->barrier = false;
      }
   }

   bool ssa_flattening = ssa_flattening_supported(entry_block);
   if (!ssa_flattening)
      entry_block = glsl_basic_block_flatten(entry_block, loop_unroll);

   /* Do the conversion to SSA form */
   SSAShader ir_sh;

   glsl_ssa_convert(&ir_sh, entry_block, &symbs->out, symbol_ids);
   glsl_basic_block_delete_reachable(entry_block);

   ssa_shader_optimise(&ir_sh, ssa_flattening);

   if (flavour == SHADER_VERTEX || flavour == SHADER_FRAGMENT || flavour == SHADER_COMPUTE)
      hoist_loads(&ir_sh);

   ret = glsl_compiled_shader_create(flavour, version);
   if (ret == NULL) return NULL;

   Map *symbol_map = glsl_map_new();
   if (!copy_compiled_shader(ret, symbs, symbol_map)) {
      glsl_compiled_shader_free(ret);
      return NULL;
   }

   if (!interface_from_symbols(&ret->in,      &symbs->in,      symbol_map, symbol_ids, false) ||
       !interface_from_symbols(&ret->out,     &symbs->out,     symbol_map, symbol_ids, false) ||
       !interface_from_symbols(&ret->uniform, &symbs->uniform, symbol_map, symbol_ids, true ) ||
       !interface_from_symbols(&ret->buffer,  &symbs->buffer,  symbol_map, symbol_ids, false)   )
   {
      glsl_compiled_shader_free(ret);
      return NULL;
   }

   glsl_map_delete(symbol_map);

   /* TODO: The vulkan driver does not currently support activity information because the
    * information in the interface variables is not complete. We skip generating for that
    * case and just leave it as it was. (Likely leaving everything marked as active) */
   if (activity_supported)
      mark_interface_actives(&ret->in, &ret->uniform, &ret->buffer, ir_sh.blocks, ir_sh.n_blocks);

   if (!copy_shader_ir(&ret->ir, &ir_sh)) {
      glsl_compiled_shader_free(ret);
      return NULL;
   }

   ret->u = *iface_data;

   if (flavour == SHADER_FRAGMENT)
      ret->u.fs.early_tests = use_early_fragment_tests(&ret->ir, ret->u.fs.early_tests);

   glsl_ssa_shader_term(&ir_sh);

   return ret;
}
