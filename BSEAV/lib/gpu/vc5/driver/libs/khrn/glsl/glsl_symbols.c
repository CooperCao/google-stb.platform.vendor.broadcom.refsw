/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "glsl_common.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "libs/util/snprintf.h"

#include "glsl_globals.h"
#include "glsl_map.h"
#include "glsl_symbols.h"
#include "glsl_ast.h"
#include "glsl_errors.h"
#include "glsl_primitive_types.auto.h"
#include "glsl_stdlib.auto.h"
#include "glsl_const_types.h"
#include "glsl_fastmem.h"
#include "glsl_intern.h"
#include "glsl_dataflow.h"
#include "glsl_symbol_table.h"

#include "glsl_ast_print.h" /* For error message printing */

/* Returns whether two types are identical. Relies on canonicalness of types, so
   only works within a shader. */
bool glsl_shallow_match_nonfunction_types(const SymbolType *a, const SymbolType *b)
{
   /* Canonical match for PRIMITIVE or STRUCT types */
   if (a == b) return true;
   else if (a->flavour == SYMBOL_ARRAY_TYPE && b->flavour == SYMBOL_ARRAY_TYPE)
   {
      // Array types are not canonical, so do a manual match.
      if (a->u.array_type.member_count != b->u.array_type.member_count)
         return false;
      /* Unsized arrays are never a match */
      if (a->u.array_type.member_count == 0)
         return false;

      return glsl_shallow_match_nonfunction_types(a->u.array_type.member_type,
                                                  b->u.array_type.member_type);
   }

   return false;
}

static bool glsl_layouts_equal(const LayoutQualifier *lq1, const LayoutQualifier *lq2)
{
   if(lq1 == lq2)
      return true;

   if(lq1->qualified != lq2->qualified)
      return false;

   if( (lq1->qualified & LOC_QUALED) && lq1->location!=lq2->location )
      return false;

   if( (lq1->qualified & UNIF_QUALED) && lq1->unif_bits!=lq2->unif_bits )
      return false;

   if( (lq1->qualified & BINDING_QUALED) && lq1->binding != lq2->binding )
      return false;

   if( (lq1->qualified & OFFSET_QUALED) && lq1->offset != lq2->offset  )
      return false;

   if( (lq1->qualified & FORMAT_QUALED) && lq1->format != lq2->format )
      return false;

   return true;
}

bool glsl_deep_match_nonfunction_types(const SymbolType *a, const SymbolType *b, bool check_prec)
{
   if (a->flavour != b->flavour)
      return false;

   switch (a->flavour)
   {
      case SYMBOL_PRIMITIVE_TYPE:
         if (a->u.primitive_type.index != b->u.primitive_type.index)
            return false;

         break;

      case SYMBOL_STRUCT_TYPE:
         if (a->u.struct_type.member_count != b->u.struct_type.member_count)
            return false;

         /* Anonymous types can never match, and type names must match */
         if (a->name == NULL || b->name == NULL || strcmp(a->name, b->name) != 0)
            return false;

         for (unsigned i = 0; i < a->u.struct_type.member_count; i++)
         {
            if (strcmp(a->u.struct_type.member[i].name, b->u.struct_type.member[i].name) != 0)
               return false;

            if (check_prec && a->u.struct_type.member[i].prec != b->u.struct_type.member[i].prec)
               return false;

            if (!glsl_deep_match_nonfunction_types(a->u.struct_type.member[i].type, b->u.struct_type.member[i].type, check_prec))
               return false;
         }
         break;

      case SYMBOL_BLOCK_TYPE:
         if (a->u.block_type.member_count != b->u.block_type.member_count)
            return false;

         if(!glsl_layouts_equal(a->u.block_type.lq, b->u.block_type.lq))
            return false;

         /* Anonymous types can never match, and type names must match */
         if (a->name == NULL || b->name == NULL || strcmp(a->name, b->name) != 0)
            return false;

         for (unsigned i = 0; i < a->u.block_type.member_count; i++)
         {
            if (strcmp(a->u.block_type.member[i].name, b->u.block_type.member[i].name) != 0)
               return false;

            if(!glsl_layouts_equal(a->u.block_type.member[i].layout, b->u.block_type.member[i].layout))
               return false;

            if (a->u.block_type.member[i].memq != b->u.block_type.member[i].memq)
               return false;

            if (check_prec &&
                a->u.block_type.member[i].prec != b->u.block_type.member[i].prec)
               return false;

            if (!glsl_deep_match_nonfunction_types(a->u.block_type.member[i].type, b->u.block_type.member[i].type, check_prec))
               return false;
         }
         break;

      case SYMBOL_ARRAY_TYPE:
         if (a->u.array_type.member_count != b->u.array_type.member_count)
            return false;

         if (!glsl_deep_match_nonfunction_types(a->u.array_type.member_type, b->u.array_type.member_type, check_prec))
            return false;
         break;

      default:
         unreachable();
         return false;
   }

   return true;
}

static inline const_value const_value_from_const_value(PRIMITIVE_TYPE_FLAGS_T to_flags, PRIMITIVE_TYPE_FLAGS_T from_flags, const_value from_value)
{
   assert(from_flags & (PRIM_BOOL_TYPE | PRIM_INT_TYPE | PRIM_UINT_TYPE | PRIM_FLOAT_TYPE));
   assert(to_flags   & (PRIM_BOOL_TYPE | PRIM_INT_TYPE | PRIM_UINT_TYPE | PRIM_FLOAT_TYPE));

   if(from_flags & PRIM_BOOL_TYPE)
   {
      switch(to_flags)
      {
         case PRIM_BOOL_TYPE:
            return from_value;
         case PRIM_INT_TYPE:
         case PRIM_UINT_TYPE:
            return (from_value ? 1 : 0);
         case PRIM_FLOAT_TYPE:
            return (from_value ? CONST_FLOAT_ONE : CONST_FLOAT_ZERO);
         default:
            unreachable();
            return 0;
      }
   }
   else if(from_flags & PRIM_INT_TYPE)
   {
      switch(to_flags)
      {
         case PRIM_BOOL_TYPE:
            return (from_value ? CONST_BOOL_TRUE : CONST_BOOL_FALSE);
         case PRIM_INT_TYPE:
         case PRIM_UINT_TYPE:
            return from_value; // bit pattern is preserved in both cases
         case PRIM_FLOAT_TYPE:
            return const_float_from_int(from_value);
         default:
            unreachable();
            return 0;
      }
   }
   else if(from_flags & PRIM_UINT_TYPE)
   {
      switch (to_flags)
      {
         case PRIM_BOOL_TYPE:
            return (from_value ? CONST_BOOL_TRUE : CONST_BOOL_FALSE);
         case PRIM_INT_TYPE:
         case PRIM_UINT_TYPE:
            return from_value; // bit pattern is preserved in both cases
         case PRIM_FLOAT_TYPE:
            return const_float_from_uint(from_value);
         default:
            unreachable();
            return 0;
      }
   }
   else /* if(from_flags & PRIM_FLOAT_TYPE) */
   {
      switch (to_flags)
      {
         case PRIM_BOOL_TYPE:
            from_value &= 0x7fffffff; // make -0.0f into 0.0f.
            return (from_value ? CONST_BOOL_TRUE : CONST_BOOL_FALSE);
         case PRIM_INT_TYPE:
            return const_int_from_float(from_value);
         case PRIM_UINT_TYPE: // behaviour undefined if from_value<0.0 and converting to uint
            return const_uint_from_float(from_value);
         case PRIM_FLOAT_TYPE:
            return from_value;
         default:
            unreachable();
            return 0;
      }
   }
}

static const PRIMITIVE_TYPE_FLAGS_T conv_valid = PRIM_BOOL_TYPE | PRIM_INT_TYPE |
                                                 PRIM_UINT_TYPE | PRIM_FLOAT_TYPE;

bool glsl_conversion_valid(PrimitiveTypeIndex from, PrimitiveTypeIndex to) {
   PRIMITIVE_TYPE_FLAGS_T from_flags = primitiveTypeFlags[from] & conv_valid;
   PRIMITIVE_TYPE_FLAGS_T to_flags   = primitiveTypeFlags[to]   & conv_valid;
   return (from_flags && to_flags);
}

const_value glsl_single_scalar_type_conversion(PrimitiveTypeIndex to_index,
                                               PrimitiveTypeIndex from_index,
                                               const_value from_val)
{
   assert(glsl_conversion_valid(from_index, to_index));

   PRIMITIVE_TYPE_FLAGS_T from_flags = primitiveTypeFlags[from_index] & conv_valid;
   PRIMITIVE_TYPE_FLAGS_T to_flags   = primitiveTypeFlags[to_index]   & conv_valid;

   return const_value_from_const_value(to_flags, from_flags, from_val);
}

/*
   Symbol *glsl_resolve_overload_using_arguments(Symbol *head, ExprChain *args)

   Function names can be overloaded. If functions' names and argument types
   match, then their return type and parameter qualifiers must also match. When
   overloaded functions are resolved, an exact match for the function's
   signature is sought -- no promotion or demotion of the return type or input
   argument types is done.
*/
static bool args_match_overload(SymbolType *overload, ExprChain *args)
{
   ExprChainNode *arg = args->first;

   if (overload->u.function_type.param_count != args->count)
      return false;

   for (unsigned i=0; i<args->count; i++, arg = arg->next)
   {
      Symbol *existing_param = overload->u.function_type.params[i];

      /* If this parameter doesn't match then give up on this overload */
      if (!glsl_shallow_match_nonfunction_types(arg->expr->type, existing_param->type))
         return false;
   }

   return true;
}

Symbol *glsl_resolve_overload_using_arguments(Symbol *head, ExprChain *args)
{
   for ( ; head; head = head->u.function_instance.next_overload) {
      assert(head->flavour == SYMBOL_FUNCTION_INSTANCE);
      assert(head->type->flavour == SYMBOL_FUNCTION_TYPE);

      if (args_match_overload(head->type, args))
         return head;
   }

   return NULL;
}

Symbol *glsl_resolve_overload_using_prototype(Symbol *head, SymbolType *prototype)
{
   assert(prototype->flavour == SYMBOL_FUNCTION_TYPE);
   unsigned param_count = prototype->u.function_type.param_count;

   for ( ; head; head = head->u.function_instance.next_overload)
   {
      SymbolType *existing_prototype = head->type;

      assert(head->flavour == SYMBOL_FUNCTION_INSTANCE);
      assert(existing_prototype->flavour == SYMBOL_FUNCTION_TYPE);

      // First check that the number of params is the same.
      if (existing_prototype->u.function_type.param_count != param_count)
         continue; // next prototype

      // Next match the param types, one by one.
      bool params_match = true;
      for (unsigned i = 0; i < param_count; i++) {
         Symbol *existing_prototype_param = existing_prototype->u.function_type.params[i];
         Symbol *prototype_param = prototype->u.function_type.params[i];

         if (!glsl_shallow_match_nonfunction_types(prototype_param->type, existing_prototype_param->type))
         {
            params_match = false;
            break; // stop matching pairs
         }
      }

      if (!params_match) continue; // next prototype

      // Under the overloading rules we've found a match, and CANNOT consider any further overloads.
      // However, there are some further tests to pass.

      // The return types must match.
      if (!glsl_shallow_match_nonfunction_types(existing_prototype->u.function_type.return_type,
                                                prototype->u.function_type.return_type) )
      {
         glsl_compile_error(ERROR_SEMANTIC, 6, g_LineNumber, "return type %s, expected %s",
                                                             prototype->u.function_type.return_type->name,
                                                             existing_prototype->u.function_type.return_type->name);
      }

      // Parameter qualifiers must be consistent between declaration and definition.
      for (unsigned i = 0; i < param_count; i++)
      {
         Symbol *existing_param = existing_prototype->u.function_type.params[i];
         Symbol *new_param = prototype->u.function_type.params[i];

         if ((new_param->u.param_instance.param_qual   != existing_param->u.param_instance.param_qual)   ||
             (new_param->u.param_instance.storage_qual != existing_param->u.param_instance.storage_qual) ||
             (new_param->u.param_instance.mem_qual     != existing_param->u.param_instance.mem_qual)     )
         {
            glsl_compile_error(ERROR_SEMANTIC, 6, g_LineNumber, "parameter %s mismatches", existing_param->name);
         }
      }

      // Success!
      return head;
   }

   return NULL;
}

bool glsl_type_contains(const SymbolType *t, PRIMITIVE_TYPE_FLAGS_T f) {
   switch (t->flavour)
   {
      case SYMBOL_PRIMITIVE_TYPE:
         return ((primitiveTypeFlags[t->u.primitive_type.index] & f) != 0);

      case SYMBOL_STRUCT_TYPE:
         for (unsigned i = 0; i < t->u.struct_type.member_count; i++) {
            if (glsl_type_contains(t->u.struct_type.member[i].type, f))
               return true;
         }
         return false;

      case SYMBOL_BLOCK_TYPE:
         for (unsigned i=0; i<t->u.block_type.member_count; i++) {
            if (glsl_type_contains(t->u.block_type.member[i].type, f))
               return true;
         }
         return false;

      case SYMBOL_ARRAY_TYPE:
         return glsl_type_contains(t->u.array_type.member_type, f);

      case SYMBOL_FUNCTION_TYPE: // fall
      default:
         unreachable();
         return false;
   }
}

bool glsl_type_contains_opaque(const SymbolType *t) {
   return glsl_type_contains(t, PRIM_IMAGE_TYPE | PRIM_SAMPLER_TYPE | PRIM_ATOMIC_TYPE);
}

bool glsl_type_contains_array(const SymbolType *t) {
   switch (t->flavour) {
      case SYMBOL_PRIMITIVE_TYPE: return false;
      case SYMBOL_ARRAY_TYPE:     return true;
      case SYMBOL_STRUCT_TYPE:
         for (unsigned i=0; i<t->u.struct_type.member_count; i++) {
            if (glsl_type_contains_array(t->u.struct_type.member[i].type)) return true;
         }
         return false;
      case SYMBOL_BLOCK_TYPE:
         for (unsigned i=0; i<t->u.block_type.member_count; i++) {
            if (glsl_type_contains_array(t->u.block_type.member[i].type)) return true;
         }
         return false;
      case SYMBOL_FUNCTION_TYPE:
      default:
         unreachable();
         return false;
   }
}

PrimitiveTypeIndex glsl_get_scalar_value_type_index(const SymbolType *type, unsigned int n)
{
   unsigned int i;
   assert(n < type->scalar_count);

   switch (type->flavour)
   {
      case SYMBOL_PRIMITIVE_TYPE:
         assert(primitiveScalarTypeIndices[type->u.primitive_type.index] != PRIMITIVE_TYPE_UNDEFINED);
         return primitiveScalarTypeIndices[type->u.primitive_type.index];

      case SYMBOL_STRUCT_TYPE:
         for (i = 0; i < type->u.struct_type.member_count; i++)
         {
            int cmp = n - type->u.struct_type.member[i].type->scalar_count;
            if (cmp < 0) break;
            n = cmp;
         }

         return glsl_get_scalar_value_type_index(type->u.struct_type.member[i].type, n);

      case SYMBOL_BLOCK_TYPE:
         for (i = 0; i < type->u.block_type.member_count; i++)
         {
            int cmp = n - type->u.block_type.member[i].type->scalar_count;
            if (cmp < 0) break;
            n = cmp;
         }

         return glsl_get_scalar_value_type_index(type->u.block_type.member[i].type, n);

      case SYMBOL_ARRAY_TYPE:
         return glsl_get_scalar_value_type_index(type->u.array_type.member_type,
                                                 n % type->u.array_type.member_type->scalar_count);

      default:
         unreachable();
         return PRIM_VOID;
   }
}

static void symbol_construct_common(Symbol *result, SymbolFlavour f, const char *name, SymbolType *type) {
   result->flavour  = f;
   result->name     = name;
   result->type     = type;
}

void glsl_symbol_construct_type(Symbol *result, SymbolType *type) {
   symbol_construct_common(result, SYMBOL_TYPE, type->name, type);
}

void glsl_symbol_construct_interface_block(Symbol *result, const char *name, SymbolType *ref_type, SymbolType *type, Qualifiers *q)
{
   symbol_construct_common(result, SYMBOL_INTERFACE_BLOCK, name, ref_type);

   result->u.interface_block.sq = q->sq;
   result->u.interface_block.iq = q->iq;
   result->u.interface_block.aq = q->aq;
   result->u.interface_block.block_data_type = type;
   result->u.interface_block.layout_loc_specified  = false;
   result->u.interface_block.layout_bind_specified = false;

   if (q->lq != NULL && (q->lq->qualified & LOC_QUALED)) {
      result->u.interface_block.layout_loc_specified  = true;
      result->u.interface_block.layout_location       = q->lq->location;
   }
   if (q->lq != NULL && (q->lq->qualified & BINDING_QUALED)) {
      result->u.interface_block.layout_bind_specified = true;
      result->u.interface_block.layout_binding        = q->lq->binding;
   }
}

void glsl_symbol_construct_var_instance(Symbol *result, const char *name, SymbolType *type, Qualifiers *q, void *compile_time_value, Symbol *block_symbol)
{
   symbol_construct_common(result, SYMBOL_VAR_INSTANCE, name, type);

   result->u.var_instance.layout_loc_specified    = false;
   result->u.var_instance.layout_bind_specified   = false;
   result->u.var_instance.offset_specified        = false;
   result->u.var_instance.layout_format_specified = false;

   if(q->lq != NULL) {
      if (q->lq->qualified & LOC_QUALED) {
         result->u.var_instance.layout_loc_specified = true;
         result->u.var_instance.layout_location      = q->lq->location;
      }

      if (q->lq->qualified & BINDING_QUALED) {
         result->u.var_instance.layout_bind_specified = true;
         result->u.var_instance.layout_binding        = q->lq->binding;
      }

      if (q->lq->qualified & OFFSET_QUALED) {
         result->u.var_instance.offset_specified = true;
         result->u.var_instance.offset           = q->lq->offset;
      }
      if (q->lq->qualified & FORMAT_QUALED) {
         result->u.var_instance.layout_format_specified = true;
         result->u.var_instance.layout_format           = q->lq->format;
      }
   }

   result->u.var_instance.interp_qual  = q->iq;
   result->u.var_instance.aux_qual     = q->aq;
   result->u.var_instance.storage_qual = q->sq;
   result->u.var_instance.prec_qual    = q->pq;
   result->u.var_instance.mem_qual     = q->mq;
   result->u.var_instance.compile_time_value = compile_time_value;
   result->u.var_instance.block_info_valid = false;

   if (block_symbol) {
      result->u.var_instance.block_info_valid = true;
      result->u.var_instance.block_info.block_symbol = block_symbol;
      result->u.var_instance.block_info.field_no = -1;

      SymbolType *block_type = block_symbol->u.interface_block.block_data_type;
      if (block_type->flavour == SYMBOL_ARRAY_TYPE) block_type = block_type->u.array_type.member_type;
      assert(block_type->u.block_type.layout->flavour==LAYOUT_STRUCT);
      if( type->flavour==SYMBOL_BLOCK_TYPE ||
          (type->flavour==SYMBOL_ARRAY_TYPE && type->u.array_type.member_type->flavour==SYMBOL_BLOCK_TYPE) )
      {
         // We are constructing the symbol for an interface block instance.
         result->u.var_instance.block_info.layout = block_type->u.block_type.layout;
         block_type->u.block_type.has_named_instance = true;
      }
      else
      {
         // the member of an anonymous block
         for(unsigned i=0; i<block_type->u.block_type.member_count; i++) {
            if(!strcmp(block_type->u.block_type.member[i].name, name)) {
               result->u.var_instance.block_info.field_no = i;
               result->u.var_instance.block_info.layout   = &block_type->u.block_type.layout->u.struct_layout.member_layouts[i];
               break;
            }
         }
      }
   } else if (q->sq == STORAGE_UNIFORM) {
      //Fill in 'block_info' for uniform instance in default block:
      result->u.var_instance.block_info_valid = true;
      result->u.var_instance.block_info.block_symbol = NULL;
      result->u.var_instance.block_info.layout = malloc_fast(sizeof(MemLayout));

      glsl_mem_calculate_non_block_layout(result->u.var_instance.block_info.layout, type);
   }
}

void glsl_symbol_construct_param_instance(Symbol *result, const char *name, SymbolType *type, StorageQualifier sq, ParamQualifier pq, MemoryQualifier mq)
{
   symbol_construct_common(result, SYMBOL_PARAM_INSTANCE, name, type);

   result->u.param_instance.storage_qual = sq;
   result->u.param_instance.param_qual   = pq;
   result->u.param_instance.mem_qual     = mq;
}

void glsl_symbol_construct_function_instance(Symbol *result, const char *name, SymbolType *type,
                                             FoldingFunction folding_function, Symbol *next_overload, bool has_prototype)
{
   symbol_construct_common(result, SYMBOL_FUNCTION_INSTANCE, name, type);

   result->u.function_instance.folding_function = folding_function;
   result->u.function_instance.function_def     = NULL;
   result->u.function_instance.has_prototype    = has_prototype;
   result->u.function_instance.next_overload    = next_overload;
}

Symbol *glsl_symbol_construct_temporary(SymbolType *type)
{
   static int temp_symbols_count = 0;

   char name[32];
   snprintf(name, 32, "$t%i", temp_symbols_count++);

   Symbol *symbol = malloc_fast(sizeof(Symbol));
   symbol_construct_common(symbol, SYMBOL_TEMPORARY, glsl_intern(name, true), type);
   return symbol;
}

Dataflow **glsl_symbol_get_default_scalar_values(const Symbol *symbol)
{
   Dataflow **scalar_values = malloc_fast(sizeof(Dataflow*) * symbol->type->scalar_count);
   for (unsigned i = 0; i < symbol->type->scalar_count; i++) {
      PrimitiveTypeIndex type_index = glsl_get_scalar_value_type_index(symbol->type, i);

      if(!glsl_prim_is_opaque_type(&primitiveTypes[type_index]))
         scalar_values[i] = glsl_dataflow_construct_const_value(glsl_prim_index_to_df_type(type_index), 0);
      else {
         if (glsl_prim_is_prim_sampler_type(&primitiveTypes[type_index])) {
            PrimSamplerInfo *psi = glsl_prim_get_sampler_info(type_index);
            PrimitiveTypeIndex ret_basic_type = primitiveScalarTypeIndices[psi->return_type];
            DataflowType type;
            switch (ret_basic_type) {
               case PRIM_FLOAT:    type = DF_F_SAMP_IMG; break;
               case PRIM_INT:      type = DF_I_SAMP_IMG; break;
               case PRIM_UINT:     type = DF_U_SAMP_IMG; break;
               default: assert(0); type = DF_INVALID;    break;
            }
            scalar_values[i++] = glsl_dataflow_construct_const_image(type, -1, false);
            scalar_values[i]   = glsl_dataflow_construct_linkable_value(DATAFLOW_CONST_SAMPLER, DF_SAMPLER, -1);
         } else if (glsl_prim_is_prim_atomic_type(&primitiveTypes[type_index]))
            scalar_values[i] = glsl_dataflow_construct_const_uint(0);
         else {
            PrimSamplerInfo *psi = glsl_prim_get_image_info(type_index);
            PrimitiveTypeIndex ret_basic_type = primitiveScalarTypeIndices[psi->return_type];
            DataflowType type;
            switch (ret_basic_type) {
               case PRIM_FLOAT:    type = DF_F_STOR_IMG; break;
               case PRIM_INT:      type = DF_I_STOR_IMG; break;
               case PRIM_UINT:     type = DF_U_STOR_IMG; break;
               default: assert(0); type = DF_INVALID;    break;
            }
            scalar_values[i] = glsl_dataflow_construct_const_image(type, -1, false);
         }
      }
   }
   return scalar_values;
}

SymbolList *glsl_symbol_list_new(void)
{
   SymbolList *res = malloc_fast(sizeof(SymbolList));
   res->head = NULL;
   res->tail = NULL;
   return res;
}

void glsl_symbol_list_append(SymbolList *list, Symbol *s)
{
   SymbolListNode *node = malloc_fast(sizeof(SymbolListNode));
   node->s = s;
   node->prev = list->tail;
   node->next = NULL;

   if (list->tail) {
      list->tail->next = node;
   } else {
      list->head = node;
   }
   list->tail = node;
}

void glsl_symbol_list_pop(SymbolList *list)
{
   assert(list->tail != NULL);
   list->tail = list->tail->prev;
   if (list->tail != NULL)
      list->tail->next = NULL;
   else
      list->head = NULL;
}

bool glsl_symbol_list_contains(const SymbolList *list, Symbol *value)
{
   for(SymbolListNode *n = list->head; n != NULL; n = n->next)
      if (n->s == value)
         return true;
   return false;
}
