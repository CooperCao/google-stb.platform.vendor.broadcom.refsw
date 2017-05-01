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

Dataflow **glsl_shader_interface_create_uniform_dataflow(const Symbol *symbol, int *ids) {
   Dataflow **df = malloc_fast(symbol->type->scalar_count * sizeof(Dataflow*));

   if (symbol->flavour == SYMBOL_VAR_INSTANCE) {
      assert(symbol->u.var_instance.storage_qual == STORAGE_UNIFORM);
      for (unsigned i = 0; i < symbol->type->scalar_count; i++) {
         PrimitiveTypeIndex type_index = glsl_get_scalar_value_type_index(symbol->type, i);
         if (glsl_prim_is_prim_sampler_type(&primitiveTypes[type_index])) {
            PrimSamplerInfo *psi = glsl_prim_get_sampler_info(type_index);
            PrimitiveTypeIndex ret_basic_type = primitiveScalarTypeIndices[psi->return_type];
            DataflowType type;
            switch (ret_basic_type) {
               case PRIM_FLOAT:    type = DF_FSAMPLER; break;
               case PRIM_INT:      type = DF_ISAMPLER; break;
               case PRIM_UINT:     type = DF_USAMPLER; break;
               default: assert(0); type = DF_INVALID;  break;
            }
            df[i] = glsl_dataflow_construct_const_image(type, ids[i],
               /* Shadow lookups are always 16-bit */
               !psi->shadow && (symbol->u.var_instance.prec_qual == PREC_HIGHP));
            assert(symbol->u.var_instance.layout_format_specified == false);
         } else if (type_index == PRIM_ATOMIC_UINT) {
            df[i] = glsl_dataflow_construct_buffer(DATAFLOW_ATOMIC_COUNTER, DF_ATOMIC,
                                                   symbol->u.var_instance.layout_binding,
                                                   symbol->u.var_instance.offset + 4*i);
            df[i] = glsl_dataflow_construct_address(df[i]);
         } else if (glsl_prim_is_prim_image_type(&primitiveTypes[type_index])) {
            PrimSamplerInfo *psi = glsl_prim_get_image_info(type_index);
            PrimitiveTypeIndex ret_basic_type = primitiveScalarTypeIndices[psi->return_type];
            DataflowType type;
            switch (ret_basic_type) {
               case PRIM_FLOAT:    type = DF_FIMAGE;  break;
               case PRIM_INT:      type = DF_IIMAGE;  break;
               case PRIM_UINT:     type = DF_UIMAGE;  break;
               default: assert(0); type = DF_INVALID; break;
            }
            df[i] = glsl_dataflow_construct_const_image(type, ids[i], (symbol->u.var_instance.prec_qual == PREC_HIGHP));
            assert(symbol->u.var_instance.layout_format_specified == true);
            df[i]->u.const_image.format_valid = true;
            df[i]->u.const_image.format = symbol->u.var_instance.layout_format;
         } else {
            df[i] = glsl_dataflow_construct_buffer(DATAFLOW_UNIFORM, glsl_prim_index_to_df_type(type_index), ids[i], 0);
         }
      }
   } else {
      assert(symbol->flavour == SYMBOL_INTERFACE_BLOCK);
      for (unsigned i = 0; i < symbol->type->scalar_count; i++)
         df[i] = glsl_dataflow_construct_buffer(DATAFLOW_UNIFORM_BUFFER, DF_UINT, ids[i], 0);
   }
   return df;
}

Dataflow **glsl_shader_interface_create_buffer_dataflow(const Symbol *symbol, int *ids) {
   Dataflow **df = malloc_fast(symbol->type->scalar_count * sizeof(Dataflow*));

   assert(symbol->flavour == SYMBOL_INTERFACE_BLOCK);
   for (unsigned i = 0; i < symbol->type->scalar_count; i++)
      df[i] = glsl_dataflow_construct_buffer(DATAFLOW_STORAGE_BUFFER, DF_UINT, ids[i], 0);

   return df;
}

static void add_iface_ids(const SymbolList *iface, bool skip_stdlib, Map *symbol_ids) {
   int id = 0;
   for (SymbolListNode *n = iface->head; n!= NULL; n=n->next) {
      if (skip_stdlib && glsl_stdlib_is_stdlib_symbol(n->s)) { continue; }

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

// Create dataflow for all shader inputs (including uniforms).
// This includes stdlib-defined ones as well as user-defined ones.
// Returns scalar value map which can be used to look up those values.
Map *glsl_shader_interfaces_create_dataflow(const ShaderInterfaces *interfaces,
                                            Map *symbol_ids)
{
   Map *scalar_values = glsl_map_new();

   // Create dataflow for stdlib uniforms and inputs
   glsl_stdlib_populate_scalar_value_map(scalar_values);

   // Create dataflow for user-defined uniforms
   for (SymbolListNode *node = interfaces->uniforms->head; node; node = node->next) {
      const Symbol *symbol = node->s;

      if (glsl_stdlib_is_stdlib_symbol(symbol)) { continue; }

      int *id = glsl_map_get(symbol_ids, symbol);
      Dataflow **df = glsl_shader_interface_create_uniform_dataflow(symbol, id);

      glsl_map_put(scalar_values, symbol, df);
   }

   for (SymbolListNode *node = interfaces->buffers->head; node; node = node->next) {
      const Symbol *symbol = node->s;
      assert(!glsl_stdlib_is_stdlib_symbol(symbol));

      int *id = glsl_map_get(symbol_ids, symbol);
      Dataflow **df = glsl_shader_interface_create_buffer_dataflow(symbol, id);

      glsl_map_put(scalar_values, symbol, df);
   }

   // Create dataflow for user-defined inputs
   for (SymbolListNode *node = interfaces->ins->head; node; node = node->next) {
      const Symbol *symbol = node->s;

      if (glsl_stdlib_is_stdlib_symbol(symbol)) { continue; }

      int *id = glsl_map_get(symbol_ids, symbol);
      Dataflow **df = malloc_fast(symbol->type->scalar_count * sizeof(Dataflow*));

      for (unsigned i = 0; i < symbol->type->scalar_count; i++) {
         PrimitiveTypeIndex type_index = glsl_get_scalar_value_type_index(symbol->type, i);
         DataflowType t = glsl_prim_index_to_df_type(type_index);
         df[i] = glsl_dataflow_construct_linkable_value(DATAFLOW_IN, t, id[i]);
      }
      glsl_map_put(scalar_values, symbol, df);
   }

   return scalar_values;
}
