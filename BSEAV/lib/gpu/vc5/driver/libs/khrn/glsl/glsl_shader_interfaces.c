/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "glsl_shader_interfaces.h"
#include "glsl_fastmem.h"
#include "glsl_symbols.h"
#include "glsl_primitive_types.auto.h"
#include "glsl_stdlib.auto.h"

ShaderInterfaces *glsl_shader_interfaces_new(void)
{
   ShaderInterfaces *res = malloc_fast(sizeof(ShaderInterfaces));
   res->uniforms = glsl_symbol_list_new();
   res->ins      = glsl_symbol_list_new();
   res->outs     = glsl_symbol_list_new();
   res->buffers  = glsl_symbol_list_new();
   res->shared   = glsl_symbol_list_new();
   return res;
}

void glsl_shader_interfaces_update(ShaderInterfaces *shader_interfaces, Symbol *symbol)
{
   StorageQualifier storage_qual;
   assert(symbol->flavour == SYMBOL_VAR_INSTANCE || symbol->flavour == SYMBOL_INTERFACE_BLOCK);

   if (symbol->flavour == SYMBOL_VAR_INSTANCE) {
      assert(!symbol->u.var_instance.block_info_valid ||
             !symbol->u.var_instance.block_info.block_symbol);

      storage_qual = symbol->u.var_instance.storage_qual;
   } else {
      storage_qual = symbol->u.interface_block.sq;
   }

   SymbolList *iface;
   switch (storage_qual) {
      case STORAGE_UNIFORM: iface = shader_interfaces->uniforms; break;
      case STORAGE_IN:      iface = shader_interfaces->ins;      break;
      case STORAGE_OUT:     iface = shader_interfaces->outs;     break;
      case STORAGE_BUFFER:  iface = shader_interfaces->buffers;  break;
      case STORAGE_SHARED:  iface = shader_interfaces->shared;   break;
      default:              iface = NULL;                        break;
   }

   if (iface != NULL)
      glsl_symbol_list_append(iface, symbol);
}

static Dataflow **buffer_dataflow(const Symbol *symbol, const int *ids) {
   Dataflow **df = malloc_fast(symbol->type->scalar_count * sizeof(Dataflow*));

   if (symbol->flavour == SYMBOL_VAR_INSTANCE) {
      assert(symbol->u.var_instance.storage_qual == STORAGE_UNIFORM);
      for (unsigned i = 0; i < symbol->type->scalar_count; i++) {
         PrimitiveTypeIndex type_index = glsl_get_scalar_value_type_index(symbol->type, i);
         if (glsl_prim_is_prim_texture_type(&primitiveTypes[type_index]) ||
             glsl_prim_is_prim_image_type(&primitiveTypes[type_index])   ||
             glsl_prim_is_prim_comb_sampler_type(&primitiveTypes[type_index]))
         {
            PrimSamplerInfo *psi = glsl_prim_get_image_info(type_index);
            PrimitiveTypeIndex ret_basic_type = primitiveScalarTypeIndices[psi->return_type];
            bool is_storage = glsl_prim_is_prim_image_type(&primitiveTypes[type_index]);
            DataflowType type;
            switch (ret_basic_type) {
               case PRIM_FLOAT:    type = is_storage ? DF_F_STOR_IMG : DF_F_SAMP_IMG; break;
               case PRIM_INT:      type = is_storage ? DF_I_STOR_IMG : DF_I_SAMP_IMG; break;
               case PRIM_UINT:     type = is_storage ? DF_U_STOR_IMG : DF_U_SAMP_IMG; break;
               default: assert(0); type = DF_INVALID;    break;
            }

            df[i] = glsl_dataflow_construct_const_image(type, ids[i],
                           /* Shadow lookups are always 16-bit */
                           !glsl_prim_sampler_is_shadow(type_index) && (symbol->u.var_instance.prec_qual == PREC_HIGHP));

            df[i]->u.const_image.format_valid = symbol->u.var_instance.layout_format_specified;
            if (symbol->u.var_instance.layout_format_specified)
               df[i]->u.const_image.format = symbol->u.var_instance.layout_format;

            if (glsl_prim_is_prim_comb_sampler_type(&primitiveTypes[type_index])) {
               /* Sampled images must be paired up with their samplers */
               assert(i+1 < symbol->type->scalar_count && type_index == glsl_get_scalar_value_type_index(symbol->type, i+1));
               i++;
               df[i] = glsl_dataflow_construct_linkable_value(DATAFLOW_CONST_SAMPLER, DF_SAMPLER, ids[i]);
            }
         } else if (type_index == PRIM_SAMPLER) {
            df[i] = glsl_dataflow_construct_linkable_value(DATAFLOW_CONST_SAMPLER, DF_SAMPLER, ids[i]);
         } else if (type_index == PRIM_ATOMIC_UINT) {
            df[i] = glsl_dataflow_construct_buffer(DATAFLOW_ATOMIC_COUNTER, DF_ATOMIC,
                                                   symbol->u.var_instance.layout_binding,
                                                   symbol->u.var_instance.offset + 4*i);
            df[i] = glsl_dataflow_construct_address(df[i]);
         } else
            df[i] = glsl_dataflow_construct_buffer(DATAFLOW_UNIFORM, glsl_prim_index_to_df_type(type_index), ids[i], 0);
      }
   } else {
      assert(symbol->flavour == SYMBOL_INTERFACE_BLOCK);
      DataflowFlavour f = (symbol->u.interface_block.sq == STORAGE_UNIFORM) ? DATAFLOW_UNIFORM_BUFFER :
                                                                              DATAFLOW_STORAGE_BUFFER;
      for (unsigned i = 0; i < symbol->type->scalar_count; i++)
         df[i] = glsl_dataflow_construct_buffer(f, DF_UINT, ids[i], 0);
   }
   return df;
}

static Dataflow **link_val_dataflow(const Symbol *symbol, const int *ids) {
   Dataflow **df = malloc_fast(symbol->type->scalar_count * sizeof(Dataflow *));

   for (unsigned i = 0; i < symbol->type->scalar_count; i++) {
      PrimitiveTypeIndex type_index = glsl_get_scalar_value_type_index(symbol->type, i);
      DataflowType t = glsl_prim_index_to_df_type(type_index);
      df[i] = glsl_dataflow_construct_linkable_value(DATAFLOW_IN, t, ids[i]);
   }
   return df;
}

Dataflow **glsl_shader_interface_create_symbol_dataflow(StorageQualifier sq, const Symbol *symbol, const int *id) {
   Dataflow **df;
   switch (sq) {
      case STORAGE_IN:      df = link_val_dataflow(symbol, id); break;
      case STORAGE_UNIFORM: df = buffer_dataflow(symbol, id);   break;
      case STORAGE_BUFFER:  df = buffer_dataflow(symbol, id);   break;
      default: unreachable();
   }
   return df;
}

static void add_iface_ids(const SymbolList *iface, bool skip_stdlib, Map *symbol_ids) {
   int id = 0;
   for (SymbolListNode *n = iface->head; n!= NULL; n=n->next) {
      if (skip_stdlib && glsl_stdlib_is_stdlib_symbol(n->s) &&
          !glsl_stdlib_is_stdlib_interface_block(n->s) &&
          n->s != glsl_stdlib_get_variable(GLSL_STDLIB_VAR__FLAT__IN__HIGHP__INT__GL_LAYER))
      {
         continue;
      }

      int *ids = malloc_fast(n->s->type->scalar_count * sizeof(int));
      for (unsigned i=0; i<n->s->type->scalar_count; i++) ids[i] = id++;

      glsl_map_put(symbol_ids, n->s, ids);
   }
}

Map *glsl_shader_interfaces_id_map(const ShaderInterfaces *interfaces) {
   Map *symbol_ids = glsl_map_new();

   add_iface_ids(interfaces->uniforms, true,  symbol_ids);
   add_iface_ids(interfaces->ins,      true,  symbol_ids);
   add_iface_ids(interfaces->outs,     false, symbol_ids);
   add_iface_ids(interfaces->buffers,  false, symbol_ids);
   return symbol_ids;
}

static void interface_create_dataflow(const SymbolList *l, StorageQualifier sq, Map *scalar_values, Map *symbol_ids) {
   for (SymbolListNode *node = l->head; node; node = node->next) {
      const Symbol *symbol = node->s;

      const int *id = glsl_map_get(symbol_ids, symbol);
      if (!id) continue;

      Dataflow **df = glsl_shader_interface_create_symbol_dataflow(sq, symbol, id);
      glsl_map_put(scalar_values, symbol, df);
   }
}

// Create dataflow for all shader inputs (including uniforms).
// This includes stdlib-defined ones as well as user-defined ones.
// Returns scalar value map which can be used to look up those values.
Map *glsl_shader_interfaces_create_dataflow(const ShaderInterfaces *interfaces, Map *symbol_ids)
{
   Map *scalar_values = glsl_map_new();

   // Create dataflow for stdlib uniforms and inputs
   glsl_stdlib_populate_scalar_value_map(scalar_values);

   interface_create_dataflow(interfaces->ins,      STORAGE_IN,      scalar_values, symbol_ids);
   interface_create_dataflow(interfaces->uniforms, STORAGE_UNIFORM, scalar_values, symbol_ids);
   interface_create_dataflow(interfaces->buffers,  STORAGE_BUFFER,  scalar_values, symbol_ids);

   return scalar_values;
}
