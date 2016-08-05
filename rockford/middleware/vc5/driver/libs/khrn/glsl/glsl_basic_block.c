/*=============================================================================
Broadcom Proprietary and Confidential. (c)2015 Broadcom.
All rights reserved.

Project  :  glsl
Module   :

FILE DESCRIPTION
=============================================================================*/

#include "glsl_basic_block.h"
#include "glsl_symbols.h"
#include "glsl_primitive_types.auto.h"

BasicBlock *glsl_basic_block_construct()
{
   BasicBlock *basic_block = malloc_fast(sizeof(BasicBlock));
   basic_block->loads = glsl_map_new();
   basic_block->scalar_values = glsl_map_new();
   basic_block->branch_cond = NULL;
   basic_block->branch_target = NULL;
   basic_block->fallthrough_target = NULL;
   basic_block->memory_head = NULL;
   basic_block->barrier = false;
   return basic_block;
}

void glsl_basic_block_list_add(BasicBlockList **list, BasicBlock *value)
{
   BasicBlockList *node = malloc_fast(sizeof(BasicBlockList));
   node->v = value;
   node->next = *list;
   *list = node;
}

void glsl_basic_block_list_pop(BasicBlockList **list)
{
   *list = (*list)->next;
}

bool glsl_basic_block_list_contains(const BasicBlockList *list, BasicBlock *value)
{
   for(; list; list = list->next)
      if (list->v == value)
         return true;
   return false;
}

int glsl_basic_block_list_count(const BasicBlockList *list)
{
   int count = 0;
   for(; list; list = list->next)
      count++;
   return count;
}

static void get_reverse_postorder_list(BasicBlockList **list, Map *seen, BasicBlock *basic_block)
{
   if (basic_block == NULL || glsl_map_get(seen, basic_block)) return;
   glsl_map_put(seen, basic_block, basic_block);
   get_reverse_postorder_list(list, seen, basic_block->fallthrough_target);
   get_reverse_postorder_list(list, seen, basic_block->branch_target);
   // prepend at the start of list (to create reverse list)
   glsl_basic_block_list_add(list, basic_block);
}

// create reverse-postorder list of all basic blocks (i.e. top-down; parent precedes children)
BasicBlockList *glsl_basic_block_get_reverse_postorder_list(BasicBlock *entry)
{
   BasicBlockList *list = NULL;
   get_reverse_postorder_list(&list, glsl_map_new(), entry);
   return list;
}

// Get the scalar values array for given symbol (or create it if it does not exist).
static Dataflow **basic_block_get_scalar_values(BasicBlock *basic_block, const Symbol *symbol, bool create_loads)
{
   Dataflow **scalar_values = glsl_map_get(basic_block->scalar_values, symbol);
   if (scalar_values)
      return scalar_values;

   scalar_values = malloc_fast(sizeof(Dataflow*) * symbol->type->scalar_count);
   glsl_map_put(basic_block->scalar_values, symbol, scalar_values);

   if (create_loads) {
      Dataflow **loads = malloc_fast(sizeof(Dataflow*) * symbol->type->scalar_count);

      for (unsigned i = 0; i < symbol->type->scalar_count; i++) {
         PrimitiveTypeIndex type_index = glsl_get_scalar_value_type_index(symbol->type, i);
         DataflowType df_type;
         if (glsl_prim_is_prim_sampler_type(&primitiveTypes[type_index])) {
            PrimSamplerInfo *psi = glsl_prim_get_sampler_info(type_index);
            PrimitiveTypeIndex ret_basic_type = primitiveScalarTypeIndices[psi->return_type];
            switch (ret_basic_type) {
               case PRIM_FLOAT:    df_type = DF_FSAMPLER; break;
               case PRIM_INT:      df_type = DF_ISAMPLER; break;
               case PRIM_UINT:     df_type = DF_USAMPLER; break;
               default: assert(0); df_type = DF_INVALID;  break;
            }
         } else if (glsl_prim_is_prim_atomic_type(&primitiveTypes[type_index])) {
            df_type = DF_INT;
         } else if (glsl_prim_is_prim_image_type(&primitiveTypes[type_index])) {
            PrimSamplerInfo *psi = glsl_prim_get_image_info(type_index);
            PrimitiveTypeIndex ret_basic_type = primitiveScalarTypeIndices[psi->return_type];
            switch (ret_basic_type) {
               case PRIM_FLOAT:    df_type = DF_FIMAGE;  break;
               case PRIM_INT:      df_type = DF_IIMAGE;  break;
               case PRIM_UINT:     df_type = DF_UIMAGE;  break;
               default: assert(0); df_type = DF_INVALID; break;
            }
         } else
            df_type = glsl_prim_index_to_df_type(type_index);

         scalar_values[i] = loads[i] = glsl_dataflow_construct_load(df_type);

         if (symbol->flavour == SYMBOL_VAR_INSTANCE)
         {
            scalar_values[i]->u.load.fmt_valid = symbol->u.var_instance.layout_format_specified;
            scalar_values[i]->u.load.fmt = symbol->u.var_instance.layout_format;
         }
         else
            scalar_values[i]->u.load.fmt_valid = false;

      }
      glsl_map_put(basic_block->loads, symbol, loads);
   }

   return scalar_values;
}

Dataflow **glsl_basic_block_get_scalar_values(BasicBlock *basic_block, const Symbol *symbol)
{
   Dataflow **scalar_values = basic_block_get_scalar_values(basic_block, symbol, true);
   return scalar_values;
}

Dataflow *glsl_basic_block_get_scalar_value(BasicBlock *basic_block, const Symbol *symbol, unsigned index)
{
   Dataflow **scalar_values = basic_block_get_scalar_values(basic_block, symbol, true);
   assert(index < symbol->type->scalar_count);
   return scalar_values[index];
}

void glsl_basic_block_set_scalar_values(BasicBlock *basic_block, const Symbol *symbol, Dataflow **values)
{
   Dataflow **scalar_values = basic_block_get_scalar_values(basic_block, symbol, false);
   if (values == scalar_values) return;   /* memcpy not defined when pointers overlap */
   memcpy(scalar_values, values, sizeof(Dataflow*) * symbol->type->scalar_count);
}

void glsl_basic_block_set_scalar_value(BasicBlock *basic_block, const Symbol *symbol, unsigned index, Dataflow *value)
{
   Dataflow **scalar_values = basic_block_get_scalar_values(basic_block, symbol, true);
   assert(index < symbol->type->scalar_count);
   scalar_values[index] = value;
}
