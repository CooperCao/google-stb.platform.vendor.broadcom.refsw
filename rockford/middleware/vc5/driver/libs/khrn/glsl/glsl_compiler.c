/*=============================================================================
Broadcom Proprietary and Confidential. (c)2014 Broadcom.
All rights reserved.

Project  :  glsl
Module   :

FILE DESCRIPTION
=============================================================================*/

#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <setjmp.h>

#include "glsl_common.h"
#include "glsl_fastmem.h"
#include "glsl_errors.h"
#include "glsl_compiler.h"
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
#include "glsl_basic_block_flatten.h"
#include "glsl_basic_block_print.h"
#include "glsl_dominators.h"
#include "glsl_trace.h"
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

#include "glsl_primitive_types.auto.h"
#include "glsl_stdlib.auto.h"

#include "../glxx/glxx_int_config.h"

#include "libs/util/gfx_util/gfx_util.h"

void glsl_term_lexer();
bool glsl_find_version(int sourcec, const char *const *sourcev, int *version);

CompiledShader *compiled_shader_create(ShaderFlavour f, int version) {
   CompiledShader *ret = malloc(sizeof(CompiledShader));
   if (ret == NULL) return NULL;

   ret->flavour = f;
   ret->version = version;

   ret->str_block           = NULL;
   ret->type_block          = NULL;
   ret->lq_block            = NULL;
   ret->symbol_block        = NULL;
   ret->struct_member_block = NULL;

   ret->blocks         = NULL;
   ret->num_cfg_blocks = 0;

   ret->outputs     = NULL;
   ret->num_outputs = 0;

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

void glsl_compiled_shader_free(CompiledShader *sh) {
   if (sh == NULL) return;

   free(sh->str_block);
   free(sh->type_block);
   free(sh->lq_block);
   free(sh->symbol_block);
   free(sh->struct_member_block);

   for (int i=0; i<sh->num_cfg_blocks; i++) {
      free(sh->blocks[i].dataflow);
      free(sh->blocks[i].outputs);
   }
   free(sh->blocks);
   free(sh->outputs);
   for (int i=0; i<sh->uniform.n_vars; i++) free(sh->uniform.var[i].ids);
   free(sh->uniform.var);
   for (int i=0; i<sh->in.n_vars; i++) free(sh->in.var[i].ids);
   free(sh->in.var);
   for (int i=0; i<sh->out.n_vars; i++) free(sh->out.var[i].ids);
   free(sh->out.var);
   for (int i=0; i<sh->buffer.n_vars; i++) free(sh->buffer.var[i].ids);
   free(sh->buffer.var);

   free(sh);
}

static int symbol_list_count(SymbolList *l) {
   int i;
   SymbolListNode *n;
   for (i=0, n = l->head; n != NULL; n = n->next, i++);
   return i;
}

struct symbol_usage {
   const Symbol *symbol;
   bool used;
};

static bool interface_from_symbols(ShaderInterface *iface, int n_vars, struct symbol_usage *l, Map *symbol_map, Map *symbol_ids)
{
   bool out_of_memory = false;

   iface->n_vars = n_vars;
   iface->var = malloc(n_vars * sizeof(InterfaceVar));
   if (iface->var == NULL) {
      iface->n_vars = 0;
      return false;
   }

   for (int i=0; i<n_vars; i++) {
      InterfaceVar *v = &iface->var[i];

      v->symbol = glsl_map_get(symbol_map, l[i].symbol);
      v->active = false;

      v->static_use = l[i].used;
      v->ids = malloc(l[i].symbol->type->scalar_count * sizeof(int));
      if (v->ids == NULL) {
         out_of_memory = true;
         continue;      /* We'll free all of these, continue ensures they are initialised */
      }

      for (unsigned j=0; j<l[i].symbol->type->scalar_count; j++) {
         v->ids[j] = -1;
      }

      int *ids = glsl_map_get(symbol_ids, l[i].symbol);
      if (ids) {
         for (unsigned j=0; j<l[i].symbol->type->scalar_count; j++)
            v->ids[j] = ids[j];

         v->active = true;
      }
   }

   return !out_of_memory;
}

struct static_usage_find {
   int count;
   struct symbol_usage *data;
};

static void find_static_use(Expr *e, void *data) {
   struct static_usage_find *d = data;

   if (e->flavour == EXPR_INSTANCE) {
      assert(e->u.instance.symbol != NULL);
      for (int i=0; i<d->count; i++) {
         if (e->u.instance.symbol == d->data[i].symbol) d->data[i].used = true;
         if (e->u.instance.symbol->flavour == SYMBOL_VAR_INSTANCE &&
             e->u.instance.symbol->u.var_instance.block_info_valid &&
             e->u.instance.symbol->u.var_instance.block_info.block_symbol == d->data[i].symbol)
         {
            d->data[i].used = true;
         }
      }
   }
}

static bool interface_from_symbol_list(ShaderInterface *iface, Statement *ast, SymbolList *l, Map *symbol_map, Map *symbol_ids)
{
   int n_vars = symbol_list_count(l);
   struct symbol_usage *symbs = glsl_safemem_malloc(n_vars * sizeof(struct symbol_usage));
   int i;
   SymbolListNode *n;
   for (i=0, n=l->head; i<n_vars; i++, n=n->next) {
      symbs[i].symbol = n->s;
      symbs[i].used = false;
   }
   struct static_usage_find d = { .count = n_vars, .data = symbs };

   glsl_statement_accept_postfix(ast, &d, NULL, find_static_use);

   bool ret = interface_from_symbols(iface, n_vars, symbs, symbol_map, symbol_ids);
   glsl_safemem_free(symbs);
   return ret;
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
      case DATAFLOW_CONST_SAMPLER:
         active->uniform[dataflow->u.const_sampler.location] = true;
         break;
      case DATAFLOW_ATOMIC_COUNTER:
         active->atomic[dataflow->u.atomic_counter.binding] |= (1 << (dataflow->u.atomic_counter.offset / 4));
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

static void record_symbol(struct shader_maps *m, Symbol *s) {
   if (glsl_map_get(m->symbol, s) != NULL) return;

   glsl_map_put(m->symbol, s, s);

   record_string(m, s->name);
   record_type(m, s->type);

   if (s->flavour == SYMBOL_INTERFACE_BLOCK) record_type(m, s->u.interface_block.block_data_type);
}

static void record_interface(struct shader_maps *m, SymbolList *iface) {
   for (SymbolListNode *l = iface->head; l != NULL; l = l->next) {
      record_symbol(m, l->s);
   }
}

static bool copy_compiled_shader(CompiledShader *sh, ShaderInterfaces *ifaces, Map *symbol_map) {
   struct shader_maps maps;
   struct shader_maps output;

   shader_maps_init(&maps);

   record_interface(&maps, ifaces->uniforms);
   record_interface(&maps, ifaces->ins);
   record_interface(&maps, ifaces->outs);
   record_interface(&maps, ifaces->buffers);

   shader_maps_init(&output);

   uintptr_t string_idx = 1;
   for (MapNode *n = maps.string->head; n != NULL; n=n->next) {
      const char *str = n->k;
      glsl_map_put(output.string, str, (void *)string_idx);
      string_idx += strlen(str) + 1;
   }

   uintptr_t type_idx = 1;
   for (MapNode *n = maps.type->head; n != NULL; n = n->next) {
      glsl_map_put(output.type, n->k, (void *)type_idx);
      type_idx++;
   }

   uintptr_t lq_idx = 1;
   for (MapNode *n = maps.lq->head; n != NULL; n = n->next) {
      glsl_map_put(output.lq, n->k, (void *)lq_idx);
      lq_idx++;
   }

   uintptr_t symbol_idx = 1;
   for (MapNode *n = maps.symbol->head; n != NULL; n = n->next) {
      glsl_map_put(output.symbol, n->k, (void *)symbol_idx);
      symbol_idx++;
   }

   /* XXX This one can start from 0 because we're using map properly */
   int struct_member_idx = 0;
   for (MapNode *n = maps.struct_members->head; n != NULL; n = n->next) {
      uintptr_t arr_len = (uintptr_t)n->v;
      int *val = malloc_fast(2*sizeof(int));
      val[0] = arr_len;
      val[1] = struct_member_idx;
      glsl_map_put(output.struct_members, n->k, val);
      struct_member_idx += arr_len;
   }

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
      return false;
   }

   for (MapNode *n = output.string->head; n != NULL; n = n->next) {
      const char *str = n->k;
      int idx = (uintptr_t)n->v;
      strcpy(&sh->str_block[idx], str);
   }

   for (MapNode *n = output.lq->head; n != NULL; n = n->next) {
      const LayoutQualifier *lq = n->k;
      int idx = (uintptr_t)n->v;
      sh->lq_block[idx] = *lq;
   }

   for (MapNode *n = output.type->head; n != NULL; n = n->next) {
      const SymbolType *t = n->k;
      int idx = (uintptr_t)n->v;
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

   for (MapNode *n = output.symbol->head; n != NULL; n = n->next) {
      const Symbol *s = n->k;
      int idx = (uintptr_t)n->v;
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

   for (MapNode *n = output.struct_members->head; n != NULL; n = n->next) {
      const StructMember *sm = n->k;
      int *v = n->v;
      for (int i=0; i<v[0]; i++) {
         int idx = v[1] + i;
         sh->struct_member_block[idx] = sm[i];
         sh->struct_member_block[idx].name = &sh->str_block[(uintptr_t)glsl_map_get(output.string, sm[i].name)];
         if (sm[i].layout != NULL)
            sh->struct_member_block[idx].layout = &sh->lq_block[(uintptr_t)glsl_map_get(output.lq, sm[i].layout)];
         sh->struct_member_block[idx].type = &sh->type_block[(uintptr_t)glsl_map_get(output.type, sm[i].type)];
      }
   }
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

struct unphi_data {
   SSABlock *blocks;
   Map *unphi;
};

static void dpostv_find_phid_constants(Dataflow *d, void *data) {
   struct unphi_data *dat = data;
   Map *unphi = dat->unphi;
   SSABlock *blocks = dat->blocks;

   if (d->flavour != DATAFLOW_PHI) return;

   Dataflow *left  = d->d.binary_op.left;
   Dataflow *right = d->d.binary_op.right;
   if (left->flavour  != DATAFLOW_EXTERNAL) return;
   if (right->flavour != DATAFLOW_EXTERNAL) return;

   Dataflow *out_left, *out_right;
   out_left = blocks[left->u.external.block].outputs[left->u.external.output];
   out_right = blocks[right->u.external.block].outputs[right->u.external.output];

   if (out_left->flavour == DATAFLOW_CONST && out_right->flavour == DATAFLOW_CONST &&
       out_left->u.constant.value == out_right->u.constant.value)
   {
      glsl_map_put(unphi, d, out_left);
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

void unphi_constants(SSABlock *blocks, int n_blocks) {
   for (int i=0; i<n_blocks; i++) {
      SSABlock *b = &blocks[i];

      struct unphi_data data;
      data.blocks = blocks;
      data.unphi = glsl_map_new();
      glsl_dataflow_visit_array(b->outputs, 0, b->n_outputs, &data, NULL, dpostv_find_phid_constants);

      glsl_dataflow_visit_array(b->outputs, 0, b->n_outputs, data.unphi, dprev_apply_opt_map, NULL);
      for (int i=0; i<b->n_outputs; i++) {
         Dataflow *new = glsl_map_get(data.unphi, b->outputs[i]);
         if (new != NULL)
            b->outputs[i] = new;
      }
   }
}

struct promote_data {
   SSABlock *blocks;
   Map *map;
};

static Dataflow *dprev_promote_constants(Dataflow *d, void *data) {
   struct promote_data *dat = data;
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
      Dataflow *promoted = glsl_dataflow_construct_linkable_value(DATAFLOW_UNIFORM, out->type);
      promoted->u.linkable_value.row = out->u.linkable_value.row;
      glsl_map_put(dat->map, d, promoted);
   }
   if (out->flavour == DATAFLOW_EXTERNAL) {
      Dataflow *promoted = glsl_dataflow_construct_external(out->type);
      promoted->u.external.block  = out->u.external.block;
      promoted->u.external.output = out->u.external.output;
      glsl_map_put(dat->map, d, promoted);
   }

   /* This has no children, so could return d or NULL here */
   return NULL;
}

void promote_constants(SSABlock *blocks, int n_blocks) {
   for (int i=0; i<n_blocks; i++) {
      SSABlock *b = &blocks[i];

      struct promote_data data;
      data.blocks = blocks;
      data.map = glsl_map_new();
      glsl_dataflow_visit_array(b->outputs, 0, b->n_outputs, &data, dprev_promote_constants, NULL);

      glsl_dataflow_visit_array(b->outputs, 0, b->n_outputs, data.map, dprev_apply_opt_map, NULL);
      for (int i=0; i<b->n_outputs; i++) {
         Dataflow *new = glsl_map_get(data.map, b->outputs[i]);
         if (new != NULL)
            b->outputs[i] = new;
      }
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
   }
}

void dpostv_find_externals(Dataflow *df, void *data) {
   if (df->flavour == DATAFLOW_EXTERNAL) glsl_dataflow_chain_append(data, df);
}

void dpostv_setup_required_components(Dataflow *df, void *data)
{
   if (df->flavour != DATAFLOW_GET_VEC4_COMPONENT) return;

   /* Set the correct bit in required_components */
   Dataflow *df_operand = df->d.unary_op.operand;
   int comp_mask = 1 << df->u.get_vec4_component.component_index;
   assert(df->u.get_vec4_component.component_index < 4);

   switch (df_operand->flavour) {
      case DATAFLOW_TEXTURE: df_operand->u.texture.required_components |= comp_mask; break;
      case DATAFLOW_FRAG_GET_COL: df_operand->u.get_col.required_components |= comp_mask; break;
      case DATAFLOW_TEXTURE_SIZE: /* Do nothing. Not sure why */ break;
      case DATAFLOW_VECTOR_LOAD: df_operand->u.vector_load.required_components |= comp_mask; break;
      default: unreachable(); break;
   }
}

static void setup_required_components(SSABlock *blocks, int n_blocks) {
   for (int i=0; i<n_blocks; i++) {
      SSABlock *b = &blocks[i];
      glsl_dataflow_visit_array(b->outputs, 0, b->n_outputs, NULL, NULL, dpostv_setup_required_components);
   }
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

static void eliminate_dead_code(SSABlock *blocks, int n_blocks, IROutput *outputs, int n_outputs)
{
   DataflowVisitor dfv;

   /* Allocate an array to store which outputs are live */
   bool **is_output = malloc_fast(n_blocks * sizeof(bool *));
   DataflowChain *externals = malloc_fast(n_blocks * sizeof(DataflowChain));
   for (int i=0; i<n_blocks; i++) {
      is_output[i] = malloc_fast(blocks[i].n_outputs * sizeof(bool));
      memset(is_output[i], 0, blocks[i].n_outputs * sizeof(bool));

      glsl_dataflow_chain_init(&externals[i]);
   }

   /* Mark as active any block outputs that are shader outputs */
   for (int i=0; i<n_outputs; i++) {
      if (outputs[i].block != -1)
         is_output[outputs[i].block][outputs[i].output] = true;
   }

   /* Pass over the IR determining which outputs are live. */
   glsl_dataflow_visitor_begin(&dfv);
   for (int i=0; i<n_blocks; i++)
      single_block_activity(&blocks[i], &dfv, is_output, externals, blocks, n_blocks);
   glsl_dataflow_visitor_end(&dfv);

   /* Actually eliminate the dead code, now that we know what it is */
   int **output_mapping = malloc_fast(n_blocks * sizeof(int *));
   for (int i=0; i<n_blocks; i++) {
      SSABlock *b = &blocks[i];

      output_mapping[i] = malloc_fast(b->n_outputs * sizeof(int));
      int out_count = 0;
      for (int j=0; j<b->n_outputs; j++) {
         if (is_output[i][j]) output_mapping[i][j] = out_count++;
         else output_mapping[i][j] = -1;
      }
      Dataflow **new_outputs = malloc_fast(out_count * sizeof(Dataflow *));
      for (int j=0; j<b->n_outputs; j++) {
         if (is_output[i][j]) new_outputs[output_mapping[i][j]] = b->outputs[j];
      }

      if (b->successor_condition != -1)
        b->successor_condition = output_mapping[i][b->successor_condition];

      b->n_outputs = out_count;
      b->outputs = new_outputs;
   }

   for (int i=0; i<n_outputs; i++) {
      if (outputs[i].block != -1)
         outputs[i].output = output_mapping[outputs[i].block][outputs[i].output];
   }

   for (int i=0; i<n_blocks; i++) {
      for (DataflowChainNode *n=externals[i].head; n; n=n->l.next) {
         Dataflow *d = n->ptr;
         d->u.external.output = output_mapping[d->u.external.block][d->u.external.output];
      }
   }
}

static Map *map_block_ids(const BasicBlockList *bl) {
   Map *block_ids = glsl_map_new();
   int block_id = 0;
   for (const BasicBlockList *b = bl; b != NULL; b=b->next) {
      int *id = malloc_fast(sizeof(int));
      *id = block_id++;
      glsl_map_put(block_ids, b->v, id);
   }
   return block_ids;
}

static int get_block_id(const Map *ids, const BasicBlock *b) {
   int *id_ptr = glsl_map_get(ids, b);
   return id_ptr[0];
}

static void get_block_info_outputs(BasicBlockList *bl, Map *block_ids, SSABlock *ssa_blocks, Map **output_maps)
{
   /* Go through the blocks sorting out outputs */
   for (BasicBlockList *bl_node = bl; bl_node != NULL; bl_node = bl_node->next)
   {
      BasicBlock *b = bl_node->v;
      int id = get_block_id(block_ids, b);
      int count = 0;

      SSABlock *b_out = &ssa_blocks[id];
      b_out->id = id;

      output_maps[b_out->id] = glsl_map_new();
      Map *output_map = output_maps[b_out->id];

      for(MapNode *out_node = b->scalar_values->head; out_node != NULL; out_node = out_node->next) {
         const Symbol *s = out_node->k;
         count += s->type->scalar_count;
      }
      if (b->memory_head != NULL) count++;
      if (b->branch_cond != NULL) count++;

      b_out->n_outputs = count;
      b_out->outputs = malloc_fast(count * sizeof(Dataflow *));

      int output_idx = 0;
      for (MapNode *out_node = b->scalar_values->head; out_node != NULL; out_node = out_node->next) {
         const Symbol *s = out_node->k;
         Dataflow **df = out_node->v;

         IROutput *o = malloc_fast(sizeof(IROutput) * s->type->scalar_count);
         for (unsigned i=0; i<s->type->scalar_count; i++) {
            b_out->outputs[output_idx] = df[i];

            /* Add the outputs to the output map */
            o[i].block = b_out->id;
            o[i].output = output_idx++;
         }
         glsl_map_put(output_map, s, o);
      }
      if (b->memory_head != NULL) b_out->outputs[output_idx++] = b->memory_head;
      if (b->branch_cond != NULL) b_out->outputs[output_idx++] = b->branch_cond;
      assert(output_idx == count);

      b_out->successor_condition = (b->branch_cond != NULL) ? output_idx - 1 : -1;
      b_out->next_true  = -1;
      b_out->next_false = -1;

      /* Remove loop condition from the end if it matches an earlier entry */
      if (b->branch_cond) {
         for (int i=0; i<b_out->n_outputs-1; i++) {
            if (b_out->outputs[i] == b_out->outputs[b_out->successor_condition]) {
               b_out->successor_condition = i;
               b_out->n_outputs--;
               break;
            }
         }
      }

      if (b->branch_target != NULL)
         b_out->next_true = get_block_id(block_ids, b->branch_target);
      if (b->fallthrough_target != NULL)
         b_out->next_false = get_block_id(block_ids, b->fallthrough_target);

      b_out->barrier = b->barrier;
   }
}

static void normalise_ir_format(BasicBlockList *bl, Map *block_ids, SSABlock *ssa_blocks, Map **output_maps, Map *symbol_ids, Map **phi_args)
{
   for (BasicBlockList *bl_node = bl; bl_node != NULL; bl_node = bl_node->next) {
      BasicBlock *b = bl_node->v;
      int id = get_block_id(block_ids, b);

      Map *load_map = glsl_map_new();

      /* Create phis and external symbols for the loads */
      for (MapNode *load_node = b->loads->head; load_node; load_node = load_node->next) {
         const Symbol *s = load_node->k;
         Dataflow **df = load_node->v;

         int *ids = glsl_map_get(symbol_ids, s);
         if (ids != NULL && (s->flavour == SYMBOL_INTERFACE_BLOCK || (s->flavour == SYMBOL_VAR_INSTANCE && s->u.var_instance.storage_qual == STORAGE_UNIFORM)))
         {
            Dataflow **v;
            if (s->flavour == SYMBOL_VAR_INSTANCE || s->u.interface_block.sq == STORAGE_UNIFORM)
               v = glsl_shader_interface_create_uniform_dataflow(s, ids);
            else
               v = glsl_shader_interface_create_buffer_dataflow(s, ids);

            for (unsigned i=0; i<s->type->scalar_count; i++) glsl_map_put(load_map, df[i], v[i]);
            continue;
         }

         for (unsigned i=0; i<s->type->scalar_count; i++) {
            /* phi_args contains, in principle, the blocks that have reaching
             * definitions of this symbol                                     */
            Dataflow *ext_res = NULL;

            /* TODO: Here we work around the stupid, hacky-in-place-update list
             * by just taking *phi_arg_val as the list. Fix the list code      */
            BasicBlockList **phi_arg_val = glsl_map_get(phi_args[id], s);
            for (BasicBlockList *r_b = *phi_arg_val; r_b; r_b=r_b->next)
            {
               Dataflow **reaching_def_values = glsl_map_get(r_b->v->scalar_values, s);
               int block_id = get_block_id(block_ids, r_b->v);
               Map *output_map = output_maps[block_id];
               IROutput *o = glsl_map_get(output_map, s);
               assert(o != NULL && o[i].block != -1 && o[i].output != -1);

               Dataflow *external = glsl_dataflow_construct_external(reaching_def_values[i]->type);
               external->u.external.block = o[i].block;
               external->u.external.output = o[i].output;
               if (ext_res == NULL) ext_res = external;
               else {
                  if (ext_res->flavour != DATAFLOW_EXTERNAL ||
                      ext_res->u.external.block  != external->u.external.block ||
                      ext_res->u.external.output != external->u.external.output)
                  {
                     ext_res = glsl_dataflow_construct_phi(ext_res, external);
                  }
               }
            }
            /* TODO: This is not the right way to solve this problem. We spuriously create
             * loads in the entry block which need to be ignored so silently drop them here.
             * The presence of DATAFLOW_LOAD in the graph later will cause enough problems. */
            if(ext_res != NULL)
               glsl_map_put(load_map, df[i], ext_res);
         }
      }

      SSABlock *bbc = &ssa_blocks[id];

      /* Substitute proper dataflow for the ridiculous LOAD constructs */
      glsl_dataflow_visit_array(bbc->outputs, 0, bbc->n_outputs, load_map, dprev_apply_opt_map, NULL);
      for (int i=0; i<bbc->n_outputs; i++) {
         if (bbc->outputs[i]->flavour == DATAFLOW_LOAD)
            bbc->outputs[i] = glsl_map_get(load_map, bbc->outputs[i]);
      }
   }
}

static void fill_ir_outputs(const SymbolList *outs, Map *symbol_final_values, Map *block_ids, Map **output_maps, Map *symbol_ids, IROutput **ir_outs, int *n_outputs) {
   int max_id = 0;
   for (SymbolListNode *n = outs->head; n != NULL; n=n->next) {
      int *ids = glsl_map_get(symbol_ids, n->s);
      if (ids == NULL) continue;

      for (unsigned i=0; i<n->s->type->scalar_count; i++) {
         if (max_id < ids[i]) max_id = ids[i];
      }
   }

   IROutput *ir_outputs = glsl_safemem_malloc( (max_id + 1) * sizeof(IROutput));
   for (int i=0; i<=max_id; i++) {
      /* Initially all outputs are uninitialised */
      ir_outputs[i].block  = -1;
      ir_outputs[i].output = -1;
   }

   for (SymbolListNode *n = outs->head; n != NULL; n=n->next) {
      int *ids = glsl_map_get(symbol_ids, n->s);
      BasicBlock *b = glsl_map_get(symbol_final_values, n->s);
      if (b == NULL) {
         /* Mark this as uninitialised in the symbol map. TODO: Possibly not needed? */
         for (unsigned i=0; i<n->s->type->scalar_count; i++) ids[i] = -1;
         continue;
      }

      int block_id = get_block_id(block_ids, b);
      Map *output_map = output_maps[block_id];
      IROutput *o = glsl_map_get(output_map, n->s);
      for (unsigned i=0; i<n->s->type->scalar_count; i++) {
         ir_outputs[ids[i]] = o[i];
      }
   }

   *ir_outs = ir_outputs;
   *n_outputs = max_id + 1;
}

static bool copy_shader_ir(CompiledShader *sh, SSABlock *blocks, int n_blocks, IROutput *ir_outputs, int n_outputs)
{
   sh->num_outputs = n_outputs;
   sh->outputs = malloc(sh->num_outputs * sizeof(IROutput));
   if (sh->num_outputs > 0 && sh->outputs == NULL) return false;

   memcpy(sh->outputs, ir_outputs, n_outputs * sizeof(IROutput));

   sh->blocks = calloc(n_blocks, sizeof(CFGBlock));
   if (!sh->blocks) return false;
   sh->num_cfg_blocks = n_blocks;

   /* Now do the actual copying out */
   for (int i=0; i<n_blocks; i++) {
      SSABlock *b = &blocks[i];
      CFGBlock *b_out = &sh->blocks[i];

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
      for (MapNode *n = b->v->scalar_values->head; n != NULL; n=n->next) {
         Dataflow **scalar_values = glsl_symbol_get_default_scalar_values(n->k);
         glsl_basic_block_set_scalar_values(entry_block, n->k, scalar_values);
      }
   }

   for (MapNode *n = initial_values->head; n != NULL; n=n->next) {
      glsl_basic_block_set_scalar_values(entry_block, n->k, n->v);
   }
}

static bool symbol_really_written(BasicBlock *b, const Symbol *s) {
   Dataflow **written = glsl_map_get(b->scalar_values, s);
   Dataflow **initial = glsl_map_get(b->loads, s);
   if (!initial) return true;

   for (unsigned int i=0; i<s->type->scalar_count; i++) {
      if (written[i] != initial[i]) return true;
   }
   return false;
}

static void prune_fake_writes(BasicBlockList *l) {
   /* Scalar values is the set of all symbols touched in a block. Drop those
    * symbols that are only read from the map by replacing it                 */
   for (BasicBlockList *b = l; b != NULL; b=b->next) {
      Map *new_scv = glsl_map_new();
      for (MapNode *mn = b->v->scalar_values->head; mn != NULL; mn = mn->next) {
         if (symbol_really_written(b->v, mn->k)) {
            glsl_map_put(new_scv, mn->k, mn->v);
         }
      }
      b->v->scalar_values = new_scv;
   }
}

static void generate_compute_variables(BasicBlock *entry_block, Map *initial_values, unsigned wg_size[3], Symbol *shared_block)
{
   Symbol *s_l_idx = glsl_stdlib_get_variable(GLSL_STDLIB_VAR__IN__UINT__GL_LOCALINVOCATIONINDEX);
   Symbol *s_l_id = glsl_stdlib_get_variable(GLSL_STDLIB_VAR__IN__UVEC3__GL_LOCALINVOCATIONID);
   Symbol *s_wg_id = glsl_stdlib_get_variable(GLSL_STDLIB_VAR__IN__UVEC3__GL_WORKGROUPID);
   Symbol *s_g_id = glsl_stdlib_get_variable(GLSL_STDLIB_VAR__IN__UVEC3__GL_GLOBALINVOCATIONID);

   // Fetch local_index, global_id[3] from TLB.
   Dataflow *tlb[4];
   glsl_dataflow_construct_frag_get_col(tlb, DF_UINT, 0, 0);

   // TLB[0] contains the local_index packed with a per-row groups index. The local_index uses a whole number of bits
   // determined by the work-group size rounded up to the next power of 2.
   uint32_t wg_num_items_p2 = gfx_next_power_of_2(wg_size[0] * wg_size[1] * wg_size[2]);

   Dataflow *wg_num_items_p2_df = glsl_dataflow_construct_const_uint(wg_num_items_p2);
   Dataflow *l_idx = glsl_dataflow_construct_binary_op(DATAFLOW_REM, tlb[0], wg_num_items_p2_df);
   Dataflow *sg_idx = glsl_dataflow_construct_binary_op(DATAFLOW_DIV, tlb[0], wg_num_items_p2_df); // group index along row
   glsl_basic_block_set_scalar_values(entry_block, s_l_idx, &l_idx);

   Dataflow *g_id[3];
   Dataflow *l_id[3];
   Dataflow *wg_id[3];
   for (unsigned i = 0; i != 3; ++i)
   {
      // Fetch base global ID from varyings, add in offset from TLB.
      g_id[i] = glsl_dataflow_construct_linkable_value(DATAFLOW_IN, DF_UINT);
      g_id[i]->u.linkable_value.row = i;
      g_id[i] = glsl_dataflow_construct_binary_op(DATAFLOW_ADD, tlb[i+1], g_id[i]);

      // Compute local and work-group IDs from global ID.
      Dataflow *wg_sz = glsl_dataflow_construct_const_uint(wg_size[i]);
      wg_id[i] = glsl_dataflow_construct_binary_op(DATAFLOW_DIV, g_id[i], wg_sz);
      l_id[i] = glsl_dataflow_construct_binary_op(DATAFLOW_REM, g_id[i], wg_sz);
   }

   glsl_basic_block_set_scalar_values(entry_block, s_g_id, g_id);
   glsl_basic_block_set_scalar_values(entry_block, s_l_id, l_id);
   glsl_basic_block_set_scalar_values(entry_block, s_wg_id, wg_id);

   // Limit shared memory use to half the L2 cache size.
   Dataflow *shared_block_start = glsl_dataflow_construct_const_uint(0u);
   unsigned const shared_block_size = shared_block->u.interface_block.block_data_type->u.block_type.layout->u.struct_layout.size;
   if (shared_block_size > 0)
   {
      unsigned const l2_size = 128*1024; // timh-todo: 256k on v3d 3.3, is version available here?
      unsigned max_concurrent_groups = shared_block_size > 0 ? l2_size / 2 / shared_block_size : ~0u;
      assert(max_concurrent_groups >= 2);
      uint32_t max_groups_per_row = (V3D_MAX_TLB_WIDTH_PX * 2u) / wg_num_items_p2;
      max_groups_per_row = gfx_umin(max_groups_per_row, max_concurrent_groups / 2u);

      // timh-todo: don't do this for single core!
      Dataflow *core_offset = glsl_dataflow_construct_nullary_op(DATAFLOW_GET_THREAD_INDEX);
      core_offset = glsl_dataflow_construct_binary_op(DATAFLOW_SHR, core_offset, glsl_dataflow_construct_const_uint(6));
      core_offset = glsl_dataflow_construct_binary_op(DATAFLOW_MUL, core_offset, glsl_dataflow_construct_const_uint(l2_size/2));

      // shared_block_start = (ycd/2 * max_groups_per_row + sg_idx) * shared_block_size
      Dataflow *ycd = glsl_dataflow_construct_nullary_op(DATAFLOW_FRAG_GET_Y_UINT);
      shared_block_start = glsl_dataflow_construct_binary_op(DATAFLOW_DIV, ycd, glsl_dataflow_construct_const_uint(2u));
      shared_block_start = glsl_dataflow_construct_binary_op(DATAFLOW_MUL, shared_block_start, glsl_dataflow_construct_const_uint(max_groups_per_row));
      shared_block_start = glsl_dataflow_construct_binary_op(DATAFLOW_ADD, shared_block_start, sg_idx);
      shared_block_start = glsl_dataflow_construct_binary_op(DATAFLOW_MUL, shared_block_start, glsl_dataflow_construct_const_uint(shared_block_size));
      shared_block_start = glsl_dataflow_construct_binary_op(DATAFLOW_ADD, shared_block_start, core_offset);
      shared_block_start = glsl_dataflow_construct_binary_op(DATAFLOW_ADD, shared_block_start, glsl_dataflow_construct_nullary_op(DATAFLOW_SHARED_PTR));
   }
   glsl_basic_block_set_scalar_values(entry_block, shared_block, &shared_block_start);
}

/* XXX: Hackily copy this here to build a shared block symbol */
static Symbol *construct_shared_block(SymbolList *members)
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
   glsl_mem_calculate_block_layout(resultType->u.block_type.layout, resultType);

   Qualifiers q = { .invariant = false,
                    .lq = NULL,
                    .sq = STORAGE_SHARED,
                    .tq = TYPE_QUAL_NONE,
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

#ifdef ANDROID
__attribute__((optimize("-O0")))
#endif
CompiledShader *glsl_compile_shader(ShaderFlavour flavour, const GLSL_SHADER_SOURCE_T *src)
{
   CompiledShader *ret = NULL;

#ifdef KHRN_SHADER_DUMP_SOURCE
   glsl_shader_source_dump(src);
#endif

   glsl_fastmem_init();
   glsl_dataflow_begin_construction();
   init_compiler_common();

   // Set long jump point in case of error.
   if (setjmp(g_ErrorHandlerEnv) != 0)
   {
      // We must be jumping back from an error.
      glsl_term_lexer(); // FIXME: This is just bad; we have no real guarantee it has even been initted
      glsl_stack_cleanup();
      glsl_fastmem_term();
      glsl_dataflow_end_construction();
      glsl_safemem_cleanup();
      glsl_compiled_shader_free(ret);
      return NULL;
   }

   // Call the parser to generate an AST from the source strings
   int version;
   if (!glsl_find_version(src->sourcec, src->sourcev, &version))
      glsl_compile_error(ERROR_PREPROCESSOR, 5, -1, "Invalid or unsupported version");

   Statement *ast = glsl_parse_ast(flavour, version, src->sourcec, src->sourcev);

   ShaderInterfaces *interfaces = glsl_shader_interfaces_new();

   if (flavour == SHADER_COMPUTE) {
      SymbolType *type = &primitiveTypes[PRIM_UVEC3];
      Qualifiers quals = { .invariant = false,
                           .lq = NULL,
                           .sq = STORAGE_IN,
                           .tq = TYPE_QUAL_NONE,
                           .pq = PREC_MEDIUMP,
                           .mq = MEMORY_NONE };
      Symbol *compute_vary = malloc_fast(sizeof(Symbol));
      glsl_symbol_construct_var_instance(compute_vary, glsl_intern("$$comp_vary", false), type, &quals, NULL, NULL);
      glsl_shader_interfaces_update(interfaces, compute_vary);
   }

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

   Symbol *shared_block = construct_shared_block(interfaces->shared);

   Map *symbol_ids = glsl_shader_interfaces_id_map(interfaces);

   /* Normalise the AST */
   NStmtList *nast = glsl_nast_build(ast);

   /* Build basic block graph */
   BasicBlock *shader_start_block = glsl_basic_block_build(nast);

   unsigned wg_size[3] = { 1, 1, 1 };
   bool early_fragment_tests = false;
   enum tess_mode tess_mode = TESS_TRIANGLES;    /* Value never used */
   enum tess_spacing tess_spacing = TESS_SPACING_EQUAL;
   bool tess_cw = false;
   bool tess_points = false;
   unsigned tess_vertices = 0;               /* Value never used */
   enum gs_out_type gs_out = GS_OUT_INVALID; /* Value never used */
   unsigned gs_n_invocations = 1;
   unsigned gs_max_vertices = 0;
   if (flavour != SHADER_VERTEX) {
      bool size_declared = false;
      bool tess_mode_declared = false;
      bool tess_vertices_declared = false;
      bool gs_out_type_declared = false;
      for (StatementChainNode *n = ast->u.ast.decls->first; n; n=n->next) {
         if (n->statement->flavour != STATEMENT_QUALIFIER_DEFAULT) continue;
         for (QualListNode *qn = n->statement->u.qualifier_default.quals->head; qn; qn=qn->next) {
            if (qn->q->flavour == QUAL_LAYOUT) {
               for (LayoutIDList *idn = qn->q->u.layout; idn; idn=idn->next) {
                  switch(idn->l->id) {
                     case LQ_SIZE_X: wg_size[0] = idn->l->argument; break;
                     case LQ_SIZE_Y: wg_size[1] = idn->l->argument; break;
                     case LQ_SIZE_Z: wg_size[2] = idn->l->argument; break;
                     case LQ_EARLY_FRAGMENT_TESTS: early_fragment_tests = true; break;
                     case LQ_VERTICES:  tess_vertices = idn->l->argument; break;
                     case LQ_ISOLINES:  tess_mode = TESS_ISOLINES;  break;
                     case LQ_TRIANGLES: tess_mode = TESS_TRIANGLES; break;
                     case LQ_QUADS:     tess_mode = TESS_QUADS;     break;
                     case LQ_SPACING_EQUAL:           tess_spacing = TESS_SPACING_EQUAL;      break;
                     case LQ_SPACING_FRACTIONAL_EVEN: tess_spacing = TESS_SPACING_FRACT_EVEN; break;
                     case LQ_SPACING_FRACTIONAL_ODD:  tess_spacing = TESS_SPACING_FRACT_ODD;  break;
                     case LQ_CW:             tess_cw = true;                      break;
                     case LQ_CCW:            tess_cw = false;                     break;
                     case LQ_POINT_MODE:     tess_points = true;                  break;
                     case LQ_POINTS:         gs_out = GS_OUT_POINTS;              break;
                     case LQ_LINE_STRIP:     gs_out = GS_OUT_LINE_STRIP;          break;
                     case LQ_TRIANGLE_STRIP: gs_out = GS_OUT_TRI_STRIP;           break;
                     case LQ_INVOCATIONS:    gs_n_invocations = idn->l->argument; break;
                     case LQ_MAX_VERTICES:   gs_max_vertices  = idn->l->argument; break;
                     default: /* Do nothing */ break;
                  }

                  if (idn->l->id == LQ_VERTICES)
                     tess_vertices_declared = true;
                  if (idn->l->id == LQ_ISOLINES || idn->l->id == LQ_TRIANGLES || idn->l->id == LQ_QUADS)
                     tess_mode_declared = true;
                  if (idn->l->id == LQ_POINTS || idn->l->id == LQ_LINE_STRIP || idn->l->id == LQ_TRIANGLE_STRIP)
                     gs_out_type_declared = true;
                  if (idn->l->id == LQ_SIZE_X || idn->l->id == LQ_SIZE_Y || idn->l->id == LQ_SIZE_Z)
                     size_declared = true;
               }
            }
         }
      }
      if (flavour == SHADER_TESS_CONTROL && !tess_vertices_declared)
         glsl_compile_error(ERROR_CUSTOM, 15, -1, "tessellation control shader requires output patch size declaration");
      if (flavour == SHADER_TESS_EVALUATION && !tess_mode_declared)
         glsl_compile_error(ERROR_CUSTOM, 15, -1, "tessellation evaluation shader requires mode declaration");
      if (flavour == SHADER_GEOMETRY && !gs_out_type_declared)
         glsl_compile_error(ERROR_CUSTOM, 15, -1, "geometry shader requires output type declaration");
      if (flavour == SHADER_GEOMETRY && (gs_max_vertices < 1 || gs_max_vertices > GLXX_CONFIG_MAX_GEOMETRY_OUTPUT_VERTICES))
         glsl_compile_error(ERROR_CUSTOM, 15, -1, "geometry shader must declare max_vertices in the range (0, %d]",
                                                  GLXX_CONFIG_MAX_GEOMETRY_OUTPUT_VERTICES);
      if (flavour == SHADER_COMPUTE && !size_declared)
         glsl_compile_error(ERROR_CUSTOM, 15, -1, "compute shader requires local size qualifiers");
   }

   BasicBlockList *l = glsl_basic_block_get_reverse_postorder_list(shader_start_block);

   if (flavour == SHADER_COMPUTE) {
      unsigned wg_n_items = wg_size[0] * wg_size[1] * wg_size[2];
      if (wg_n_items <= 16 && (wg_n_items & (wg_n_items - 1)) == 0) {
         for (BasicBlockList *n = l; n != NULL; n=n->next)
            n->v->barrier = false;
      }
   }

   Map *offsets = glsl_map_new();
   int c = glsl_basic_block_list_count(l);
   for ( ; l != NULL; l=l->next) {
      int *offset = malloc_fast(sizeof(int));
      *offset = 1000;
      glsl_map_put(offsets, l->v, offset);
   }

   /* XXX We skip this code flattening for massive numbers of blocks because it is slow */
   if (c < 1000)
      while (glsl_basic_block_flatten_a_bit(shader_start_block, offsets)) /* Do nothing */;

   /* Add interface definitions to a basic block prior to the shader */
   BasicBlock *entry_block = glsl_basic_block_construct();
   int *entry_age_offset = malloc_fast(sizeof(int));
   *entry_age_offset = 0;
   glsl_map_put(offsets, entry_block, entry_age_offset);
   entry_block->fallthrough_target = shader_start_block;
   Map *initial_values = glsl_shader_interfaces_create_dataflow(interfaces, symbol_ids);

   initialise_interface_symbols(entry_block, initial_values);
   if (flavour == SHADER_COMPUTE)
      generate_compute_variables(entry_block, initial_values, wg_size, shared_block);

   BasicBlock *flattened_entry_block = glsl_basic_block_flatten(entry_block, offsets);

   /* Do the conversion to SSA form */
   SSABlock *ssa_blocks;
   int n_blocks;
   IROutput *ir_outputs;
   int n_outputs;

   {
      BasicBlockList *bl = glsl_basic_block_get_reverse_postorder_list(flattened_entry_block);
      prune_fake_writes(bl);

      n_blocks   = glsl_basic_block_list_count(bl);
      ssa_blocks = glsl_safemem_malloc(n_blocks * sizeof(SSABlock));

      Map **output_maps = glsl_safemem_malloc(n_blocks * sizeof(Map *));
      Map **phi_args    = glsl_safemem_malloc(n_blocks * sizeof(Map *));

      Map *symbol_final_values = glsl_construct_dominator_tree(bl, phi_args);
      Map *block_ids = map_block_ids(bl);
      get_block_info_outputs(bl, block_ids, ssa_blocks, output_maps);
      normalise_ir_format(bl, block_ids, ssa_blocks, output_maps, symbol_ids, phi_args);

      fill_ir_outputs(interfaces->outs, symbol_final_values, block_ids, output_maps, symbol_ids, &ir_outputs, &n_outputs);

      glsl_safemem_free(phi_args);
      glsl_safemem_free(output_maps);
   }

   unphi_constants(ssa_blocks, n_blocks);
   promote_constants(ssa_blocks, n_blocks);
   simplify_dataflow(ssa_blocks, n_blocks);

   eliminate_dead_code(ssa_blocks, n_blocks, ir_outputs, n_outputs);

   glsl_dataflow_cse(ssa_blocks, n_blocks);

   setup_required_components(ssa_blocks, n_blocks);

   ret = compiled_shader_create(flavour, version);
   if (ret == NULL) { return NULL; }

   Map *symbol_map = glsl_map_new();
   if (!copy_compiled_shader(ret, interfaces, symbol_map)) {
      glsl_compiled_shader_free(ret);
      return NULL;
   }

   if (!interface_from_symbol_list(&ret->uniform, ast, interfaces->uniforms, symbol_map, symbol_ids) ||
       !interface_from_symbol_list(&ret->in,      ast, interfaces->ins,      symbol_map, symbol_ids) ||
       !interface_from_symbol_list(&ret->out,     ast, interfaces->outs,     symbol_map, symbol_ids) ||
       !interface_from_symbol_list(&ret->buffer,  ast, interfaces->buffers,  symbol_map, symbol_ids)   )
   {
      glsl_compiled_shader_free(ret);
      return NULL;
   }

   mark_interface_actives(&ret->in, &ret->uniform, &ret->buffer, ssa_blocks, n_blocks);

   if (!copy_shader_ir(ret, ssa_blocks, n_blocks, ir_outputs, n_outputs)) {
      glsl_compiled_shader_free(ret);
      return NULL;
   }

   if (flavour == SHADER_FRAGMENT) {
      InterfaceVar *frag_color = interface_var_find(&ret->out, "gl_FragColor");
      InterfaceVar *frag_data  = interface_var_find(&ret->out, "gl_FragData");

      if (frag_color && frag_data && frag_color->static_use && frag_data->static_use)
         glsl_compile_error(ERROR_CUSTOM, 5, -1, "Use of both gl_FragColor and gl_FragData");

      InterfaceVar *frag_depth = interface_var_find(&ret->out, "gl_FragDepth");
      if (early_fragment_tests && frag_depth->static_use)
         glsl_compile_error(ERROR_CUSTOM, 5, -1, "Use of gl_FragDepth while early_fragment_tests specified");

      bool visible_effects = false;
      for (int i=0; i<ret->num_cfg_blocks; i++) {
         CFGBlock *b = &ret->blocks[i];
         for (int j=0; j<b->num_dataflow; j++) {
            if (glsl_dataflow_affects_memory(b->dataflow[j].flavour)) visible_effects = true;
         }
      }
      ret->early_fragment_tests = early_fragment_tests || !visible_effects;
   }

   if (flavour == SHADER_TESS_CONTROL) {
      ret->tess_vertices = tess_vertices;
   }

   if (flavour == SHADER_TESS_EVALUATION) {
      ret->tess_mode       = tess_mode;
      ret->tess_spacing    = tess_spacing;
      ret->tess_cw         = tess_cw;
      ret->tess_point_mode = tess_points;
   }

   if (flavour == SHADER_GEOMETRY) {
      ret->gs_out = gs_out;
      ret->gs_n_invocations = gs_n_invocations;
   }

   if (flavour == SHADER_COMPUTE) {
      for (int i=0; i<3; i++) ret->wg_size[i] = wg_size[i];
      ret->shared_block_size = shared_block->u.interface_block.block_data_type->u.block_type.layout->u.struct_layout.size;
   }

   glsl_safemem_free(ssa_blocks);
   glsl_safemem_free(ir_outputs);

   glsl_dataflow_end_construction();
   glsl_fastmem_term();
#ifndef NDEBUG
   glsl_safemem_verify();
#endif

   return ret;
}
