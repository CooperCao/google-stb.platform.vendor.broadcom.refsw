/*=============================================================================
Copyright (c) 2014 Broadcom Europe Limited.
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
#include "glsl_uniform_layout.h"
#include "glsl_ir_program.h"
#include "glsl_dataflow_builder.h"
#include "glsl_stackmem.h"
#include "glsl_dataflow_cse.h"

#include "glsl_primitive_types.auto.h"
#include "glsl_stdlib.auto.h"

extern void glsl_term_lexer();

#include "glsl_compiled_shader.h"

void glsl_init_compiler_common(void)
{
   // Initialize string intern facility
   glsl_init_intern(1024);

   glsl_dataflow_reset_count();
   glsl_prim_init();
   glsl_stdlib_init();
}

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

static bool interface_from_symbol_list(ShaderInterface *iface, SymbolList *l, Map *symbol_map, Map *symbol_ids)
{
   int i;
   SymbolListNode *n;
   bool out_of_memory = false;

   iface->n_vars = symbol_list_count(l);
   iface->var = malloc(iface->n_vars * sizeof(InterfaceVar));
   if (iface->var == NULL) {
      iface->n_vars = 0;
      return false;
   }

   for (i=0, n=l->head; i<iface->n_vars; i++, n = n->next) {
      InterfaceVar *v = &iface->var[i];

      v->symbol = glsl_map_get(symbol_map, n->s);
      v->active = false;

      v->static_use = false;
      v->ids = malloc(n->s->type->scalar_count * sizeof(int));
      if (v->ids == NULL) {
         out_of_memory = true;
         continue;      /* We'll free all of these, continue ensures they are initialised */
      }

      for (unsigned j=0; j<n->s->type->scalar_count; j++) {
         v->ids[j] = -1;
      }

      int *ids = glsl_map_get(symbol_ids, n->s);
      if (ids) {
         for (unsigned j=0; j<n->s->type->scalar_count; j++)
            v->ids[j] = ids[j];

         v->active = true;
      }
   }

   return !out_of_memory;
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

/* TODO: This tests static use, not static assignments */
struct symbol_usage {
   const Symbol *symbol;
   bool used;
};

static void find_frag_assignments(Expr *e, void *data) {
   struct symbol_usage *usage = data;

   if (e->flavour == EXPR_INSTANCE) {
      assert(e->u.instance.symbol != NULL);
      for (int i=0; i<3; i++) {
         if (e->u.instance.symbol == usage[i].symbol) usage[i].used = true;
      }
   }
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
         if ((v->ids[j] != -1) && (actives[v->ids[j]]))
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
         record_lq(m, t->u.block_type.lq);
         return;
      default: UNREACHABLE();
   }
}

static void record_symbol(struct shader_maps *m, Symbol *s) {
   if (glsl_map_get(m->symbol, s) != NULL) return;

   glsl_map_put(m->symbol, s, s);

   record_string(m, s->name);
   record_type(m, s->type);
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
            dest->u.block_type.lq = &sh->lq_block[(uintptr_t)glsl_map_get(output.lq, t->u.block_type.lq)];
            dest->u.block_type.layout = NULL;
            break;
         default:
            UNREACHABLE();
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
      dest->u.var_instance.compile_time_value = NULL;
      dest->u.var_instance.block_info.block_symbol = NULL;
      dest->u.var_instance.block_info.layout = NULL;

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
      default: UNREACHABLE(); break;
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
      if (b->branch_cond != NULL) b_out->outputs[output_idx++] = b->branch_cond;
      assert(output_idx == count);

      b_out->successor_condition = (b->branch_cond != NULL) ? output_idx - 1 : -1;
      b_out->next_true  = -1;
      b_out->next_false = -1;

      if (b->branch_target != NULL)
         b_out->next_true = get_block_id(block_ids, b->branch_target);
      if (b->fallthrough_target != NULL)
         b_out->next_false = get_block_id(block_ids, b->fallthrough_target);
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

#ifdef ANDROID
__attribute__((optimize("-O0")))
#endif
CompiledShader *glsl_compile_shader(ShaderFlavour flavour, const GLSL_SHADER_SOURCE_T *src)
{
   CompiledShader *ret = NULL;

   bool frag_color_static_use = false;
   bool frag_data_static_use  = false;
   bool frag_depth_static_use = false;

#ifdef KHRN_SHADER_DUMP_SOURCE
   glsl_shader_source_dump(src);
#endif

   glsl_fastmem_init();
   glsl_dataflow_begin_construction();
   glsl_init_compiler_common();

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
   Statement *ast = glsl_parse_ast(flavour, src->sourcec, src->sourcev);
   glsl_ast_validate(ast, flavour);

   if (flavour == SHADER_FRAGMENT)
   {
      const char *(names[3]) = { "gl_FragColor", "gl_FragData", "gl_FragDepth" };
      struct symbol_usage usage[3] = { { NULL, false }, { NULL, false }, { NULL, false } };
      for (const SymbolListNode *n = g_ShaderInterfaces->outs->head; n != NULL; n=n->next) {
         for (int i=0; i<3; i++) {
            if (strcmp(n->s->name, names[i]) == 0)
               usage[i].symbol = n->s;
         }
      }

      glsl_statement_accept_postfix(ast, usage, NULL, find_frag_assignments);

      if (usage[0].used) frag_color_static_use = true;
      if (usage[1].used) frag_data_static_use  = true;
      if (usage[2].used) frag_depth_static_use = true;
   }

   if (frag_color_static_use && frag_data_static_use)
      glsl_compile_error(ERROR_CUSTOM, 5, -1, NULL);

   Map *symbol_ids = glsl_shader_interfaces_id_map(g_ShaderInterfaces);

   /* Normalise the AST */
   NStmtList *nast = glsl_nast_build(ast);

   /* Build basic block graph */
   BasicBlock *shader_start_block = glsl_basic_block_build(nast);

   Map *offsets = glsl_map_new();
   BasicBlockList *l = glsl_basic_block_get_reverse_postorder_list(shader_start_block);
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
   Map *initial_values = glsl_shader_interfaces_create_dataflow(g_ShaderInterfaces, symbol_ids);

   initialise_interface_symbols(entry_block, initial_values);
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

      fill_ir_outputs(g_ShaderInterfaces->outs, symbol_final_values, block_ids, output_maps, symbol_ids, &ir_outputs, &n_outputs);

      glsl_safemem_free(phi_args);
      glsl_safemem_free(output_maps);
   }

   unphi_constants(ssa_blocks, n_blocks);
   promote_constants(ssa_blocks, n_blocks);
   simplify_dataflow(ssa_blocks, n_blocks);

   eliminate_dead_code(ssa_blocks, n_blocks, ir_outputs, n_outputs);

   glsl_dataflow_cse(ssa_blocks, n_blocks);

   setup_required_components(ssa_blocks, n_blocks);

   ret = compiled_shader_create(flavour, g_ShaderVersion);
   if (ret == NULL) { return NULL; }

   Map *symbol_map = glsl_map_new();
   if (!copy_compiled_shader(ret, g_ShaderInterfaces, symbol_map)) {
      glsl_compiled_shader_free(ret);
      return NULL;
   }

   if (!interface_from_symbol_list(&ret->uniform, g_ShaderInterfaces->uniforms, symbol_map, symbol_ids) ||
       !interface_from_symbol_list(&ret->in,      g_ShaderInterfaces->ins,      symbol_map, symbol_ids) ||
       !interface_from_symbol_list(&ret->out,     g_ShaderInterfaces->outs,     symbol_map, symbol_ids) ||
       !interface_from_symbol_list(&ret->buffer,  g_ShaderInterfaces->buffers,  symbol_map, symbol_ids)   )
   {
      glsl_compiled_shader_free(ret);
      return NULL;
   }

   mark_interface_actives(&ret->in, &ret->uniform, &ret->buffer, ssa_blocks, n_blocks);

   if (!copy_shader_ir(ret, ssa_blocks, n_blocks, ir_outputs, n_outputs)) {
      glsl_compiled_shader_free(ret);
      return NULL;
   }

   if (flavour == SHADER_FRAGMENT)
   {
      InterfaceVar *frag_color = interface_var_find(&ret->out, "gl_FragColor");
      InterfaceVar *frag_data  = interface_var_find(&ret->out, "gl_FragData");
      InterfaceVar *frag_depth = interface_var_find(&ret->out, "gl_FragDepth");

      if (frag_color) frag_color->static_use = frag_color_static_use;
      if (frag_data)  frag_data ->static_use = frag_data_static_use;
      if (frag_depth) frag_depth->static_use = frag_depth_static_use;
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
