/*=============================================================================
Broadcom Proprietary and Confidential. (c)2014 Broadcom.
All rights reserved.

Project  :  glsl
Module   :

FILE DESCRIPTION
=============================================================================*/

#include <stdlib.h>

#include "glsl_nast.h"
#include "glsl_common.h"
#include "glsl_dataflow.h"
#include "glsl_dataflow_builder.h"
#include "glsl_basic_block.h"
#include "glsl_basic_block_flatten.h"
#include "glsl_basic_block_print.h"
#include "glsl_intrinsic_ir_builder.h"
#include "glsl_stackmem.h"
#include "glsl_trace.h"
#include "glsl_ast_visitor.h"
#include "glsl_errors.h"
#include "glsl_fastmem.h"
#include "glsl_const_operators.h"
#include "glsl_primitive_types.auto.h"
#include "glsl_stdlib.auto.h"
#include "glsl_symbol_table.h"

// Populates an array of Dataflow* with indices into a symbol's scalar data.
// This represents the lvalue of the expr and should be updated if this expr is used as an lvalue.

static inline Dataflow **alloc_dataflow(size_t amount) {
   return glsl_stack_malloc(sizeof(Dataflow *), amount);
}

static inline void free_dataflow(Dataflow **dflow) {
   glsl_stack_free(dflow);
}

static DataflowType df_scalar_type(SymbolType *type, unsigned int scalar_idx)
{
   return glsl_prim_index_to_df_type(glsl_get_scalar_value_type_index(type, scalar_idx));
}

// Writes dataflow graph pointers for compile_time_value to scalar_values.
// scalar_values is array of Dataflow* with expr->type->scalar_count members.
static void expr_calculate_dataflow_compile_time_value(Dataflow **scalar_values, const_value *compile_time_value, SymbolType *type)
{
   for (unsigned i = 0; i < type->scalar_count; i++) {
      DataflowType t = df_scalar_type(type, i);
      scalar_values[i] = glsl_dataflow_construct_const_value(t, compile_time_value[i]);
   }
}

static void do_store(BasicBlock *ctx, Dataflow *address, Dataflow *offset, uint8_t *swizzle, Dataflow **values, unsigned count, int stride) {
   uint8_t no_swizzle[4] = { 0, 1, 2, 3 };
   if (swizzle == NULL) swizzle = no_swizzle;

   Dataflow *v[4] = { NULL, };
   assert(count <= 4);
   for (unsigned i=0; i<count; i++) v[swizzle[i]] = values[i];

   int start = 0;
   while (true) {
      while (start < 4 && v[start] == NULL) start++;
      if (start == 4) break;

      /* Store as vectors where possible (continuous and sufficiently aligned) */
      int count = 1;
      if ((start & 1) == 0 && stride == 4)
         while (start + count < 4 && v[start+count] != NULL) count++;

      Dataflow *vec[4] = { NULL, };
      for (int i=0; i<count; i++) vec[i] = v[start + i];

      Dataflow *o = glsl_dataflow_construct_binary_op(DATAFLOW_ADD, offset, glsl_dataflow_construct_const_int(stride * start));
      Dataflow *addr = glsl_dataflow_construct_binary_op(DATAFLOW_ADD, address, o);
      Dataflow *vs = glsl_dataflow_construct_vec4(vec[0], vec[1], vec[2], vec[3]);
      Dataflow *s = glsl_dataflow_construct_atomic(DATAFLOW_ADDRESS_STORE, DF_VOID, addr, vs, NULL, ctx->memory_head);
      ctx->memory_head = s;

      start += count;
   }
}

static void build_stores(BasicBlock *ctx, Dataflow *address, Dataflow *offset, uint8_t *swizzle,
                         Dataflow **values, MEMBER_LAYOUT_T *layout, SymbolType *type)
{
   switch(layout->flavour)
   {
      case MEMBER_STRUCT:
         {
            int member_scalar_offset = 0;

            assert(type->flavour==SYMBOL_STRUCT_TYPE);

            for(unsigned i=0; i<layout->u.struct_layout.member_count; i++)
            {
               int member_addr_offset = layout->u.struct_layout.member_layouts[i].level_offset;
               Dataflow *member_offset = glsl_dataflow_construct_binary_op(DATAFLOW_ADD, offset, glsl_dataflow_construct_const_int(member_addr_offset));

               build_stores(ctx, address, member_offset, swizzle, values + member_scalar_offset, &layout->u.struct_layout.member_layouts[i], type->u.struct_type.member[i].type);
               member_scalar_offset += type->u.struct_type.member[i].type->scalar_count;
            }
            break;
         }
      case MEMBER_ARRAY:
         {
            int array_stride  = layout->u.array_layout.stride;
            int member_scalars = type->u.array_type.member_type->scalar_count;
            int member_offset = 0;

            assert(type->flavour==SYMBOL_ARRAY_TYPE);

            for(unsigned i=0; i<layout->u.array_layout.member_count; i++)
            {
               Dataflow *new_off = glsl_dataflow_construct_binary_op(DATAFLOW_ADD, offset, glsl_dataflow_construct_const_int(member_offset));
               build_stores(ctx, address, new_off, swizzle, values + i*member_scalars, &layout->u.array_layout.member_layouts[i], type->u.array_type.member_type);
               member_offset += array_stride;
            }
         }
         break;
      case MEMBER_PRIMITIVE_MATRIX:
         {
            bool column_major = layout->u.matrix_layout.column_major;
            int matrix_stride = layout->u.matrix_layout.stride;
            unsigned int n_vecs, vec_size;

            assert(type->flavour==SYMBOL_PRIMITIVE_TYPE);
            assert(primitiveTypeFlags[type->u.primitive_type.index] & PRIM_MATRIX_TYPE);

            unsigned columns = glsl_prim_matrix_type_subscript_dimensions(type->u.primitive_type.index, 0);
            unsigned rows    = glsl_prim_matrix_type_subscript_dimensions(type->u.primitive_type.index, 1);

            assert(type->scalar_count==columns*rows);

            /* Consider the matrix as n_vecs vectors, each vec_size long */
            if(column_major) {
               n_vecs = columns;    /* Column vectors */
               vec_size = rows;
            } else {
               n_vecs = rows;       /* Row vectors */
               vec_size = columns;
            }

            Dataflow *out_vals[4][4];
            for (unsigned i=0; i<n_vecs; i++) {
               for (unsigned j=0; j<vec_size; j++) out_vals[i][j] = column_major ? values[i*vec_size + j] : values[j*n_vecs + i];
            }

            for(unsigned i=0; i<n_vecs; i++) {
               Dataflow *vec_offset = glsl_dataflow_construct_binary_op(DATAFLOW_ADD, offset, glsl_dataflow_construct_const_int(i*matrix_stride));
               do_store(ctx, address, vec_offset, swizzle, &out_vals[i][0], vec_size, 4);
            }
         }
         break;
      case MEMBER_PRIMITIVE_NONMATRIX:
         {
            assert(type->flavour==SYMBOL_PRIMITIVE_TYPE);
            Dataflow *v[4] = { NULL, };
            for (unsigned i=0; i<type->scalar_count; i++) {
               if (primitiveTypeFlags[type->u.primitive_type.index] & PRIM_BOOL_TYPE)
                  v[i] = glsl_dataflow_convert_type(values[i], DF_INT);
               else
                  v[i] = values[i];
            }
            int vector_stride = layout->u.vector_layout.stride;
            do_store(ctx, address, offset, swizzle, v, type->scalar_count, vector_stride);
         }
         break;

   default:
      UNREACHABLE();
   }
}

static void build_loads(Dataflow **scalar_offsets, Dataflow *base, Dataflow *offset, uint8_t *swizzle,
                        MEMBER_LAYOUT_T *layout, SymbolType *type)
{
   /* TODO: We only get into buffer addressing code from a field selector (which can't have swizzle)
    * or a dynamic index (which would fold the swizzle into the address calculation), so this can't
    * happen. It would actually be nice if it could, because then things like foo = vec_load.w wouldn't
    * be so awful */
   assert(swizzle == NULL);

   switch(layout->flavour)
   {
      case MEMBER_STRUCT:
         {
            int member_scalar_offset = 0;

            assert(type->flavour==SYMBOL_STRUCT_TYPE);

            for(unsigned i=0; i<layout->u.struct_layout.member_count; i++)
            {
               int member_addr_offset = layout->u.struct_layout.member_layouts[i].level_offset;
               Dataflow *member_offset = glsl_dataflow_construct_binary_op(DATAFLOW_ADD, offset, glsl_dataflow_construct_const_int(member_addr_offset));

               build_loads(scalar_offsets + member_scalar_offset, base, member_offset, swizzle, &layout->u.struct_layout.member_layouts[i], type->u.struct_type.member[i].type);
               member_scalar_offset += type->u.struct_type.member[i].type->scalar_count;
            }
            break;
         }
      case MEMBER_ARRAY:
         {
            int array_stride  = layout->u.array_layout.stride;
            int member_scalars = type->u.array_type.member_type->scalar_count;
            int member_offset = 0;

            assert(type->flavour==SYMBOL_ARRAY_TYPE);

            for(unsigned i=0; i<layout->u.array_layout.member_count; i++)
            {
               Dataflow *new_off = glsl_dataflow_construct_binary_op(DATAFLOW_ADD, offset, glsl_dataflow_construct_const_int(member_offset));

               build_loads(scalar_offsets + i * member_scalars, base, new_off, swizzle, &layout->u.array_layout.member_layouts[i], type->u.array_type.member_type);
               member_offset += array_stride;
            }
         }
         break;
      case MEMBER_PRIMITIVE_MATRIX:
         {
            bool column_major = layout->u.matrix_layout.column_major;
            int matrix_stride = layout->u.matrix_layout.stride;
            unsigned int n_vecs, vec_size;
            Dataflow *matrix_scalars[4][4];

            assert(type->flavour==SYMBOL_PRIMITIVE_TYPE);
            assert(primitiveTypeFlags[type->u.primitive_type.index] & PRIM_MATRIX_TYPE);

            unsigned columns = glsl_prim_matrix_type_subscript_dimensions(type->u.primitive_type.index, 0);
            unsigned rows    = glsl_prim_matrix_type_subscript_dimensions(type->u.primitive_type.index, 1);

            assert(type->scalar_count==columns*rows);

            /* Consider the matrix as n_vecs vectors, each vec_size long */
            if(column_major) {
               n_vecs = columns;    /* Column vectors */
               vec_size = rows;
            } else {
               n_vecs = rows;       /* Row vectors */
               vec_size = columns;
            }

            for(unsigned i=0; i<n_vecs; i++)
            {
               Dataflow *vec_offset = glsl_dataflow_construct_binary_op(DATAFLOW_ADD, offset, glsl_dataflow_construct_const_int(i*matrix_stride));
               Dataflow *vec_address = glsl_dataflow_construct_binary_op(DATAFLOW_ADD, base, vec_offset);
               Dataflow *vec = glsl_dataflow_construct_vector_load(df_scalar_type(type, 0), vec_address);

               for(unsigned j=0; j<vec_size; j++)
                  matrix_scalars[i][j] = glsl_dataflow_construct_get_vec4_component(j, vec, df_scalar_type(type, 0));
            }

            for (unsigned i=0; i<n_vecs; i++) {
               for (unsigned j=0; j<vec_size; j++) {
                  int scalar_idx;
                  /* Scalar offsets is column major, so row major transposes */
                  if (column_major) scalar_idx = i*rows + j;
                  else              scalar_idx = j*rows + i;

                  scalar_offsets[scalar_idx] = matrix_scalars[i][j];
               }
            }
         }
         break;
      case MEMBER_PRIMITIVE_NONMATRIX:
         {
            int vector_stride = 0;
            int member_offset = 0;

            assert(type->flavour==SYMBOL_PRIMITIVE_TYPE);

            if(primitiveTypeFlags[type->u.primitive_type.index] & PRIM_VECTOR_TYPE)
               vector_stride = layout->u.vector_layout.stride;

            if (vector_stride != 4) {
               for(unsigned i=0; i<type->scalar_count; i++)
               {
                  Dataflow *o = glsl_dataflow_construct_binary_op(DATAFLOW_ADD, offset, glsl_dataflow_construct_const_int(member_offset));
                  Dataflow *address = glsl_dataflow_construct_binary_op(DATAFLOW_ADD, base, o);
                  scalar_offsets[i] = glsl_dataflow_construct_address_load(df_scalar_type(type, i),
                                                                           address);
                  member_offset += vector_stride;
               }
            } else {
               Dataflow *vec_addr = glsl_dataflow_construct_binary_op(DATAFLOW_ADD, base, offset);
               Dataflow *vec = glsl_dataflow_construct_vector_load(df_scalar_type(type, 0), vec_addr);
               for (unsigned i=0; i<type->scalar_count; i++) {
                  scalar_offsets[i] = glsl_dataflow_construct_get_vec4_component(i, vec, df_scalar_type(type, 0));
               }
            }
         }
         break;

   default:
      UNREACHABLE();
   }
}

static Dataflow *construct_bounds_check(Dataflow *subscript, int size)
{
   /* All subscripts should have been converted to int by now */
   subscript = glsl_dataflow_construct_binary_op(DATAFLOW_MAX, subscript, glsl_dataflow_construct_const_int(0));
   return      glsl_dataflow_construct_binary_op(DATAFLOW_MIN, subscript, glsl_dataflow_construct_const_int(size-1));
}

static void combine_address_swizzle(uint8_t **swizzle, uint8_t *then) {
   if (*swizzle == NULL) {
      *swizzle = malloc_fast(sizeof(uint8_t) * MAX_SWIZZLE_FIELD_COUNT);
      for (int i=0; i<MAX_SWIZZLE_FIELD_COUNT; i++) {
         (*swizzle)[i] = then[i];
      }
   } else {
      uint8_t first[MAX_SWIZZLE_FIELD_COUNT];
      for (int i=0; i<MAX_SWIZZLE_FIELD_COUNT; i++) first[i] = (*swizzle)[i];
      for (int i=0; i<MAX_SWIZZLE_FIELD_COUNT; i++) {
         (*swizzle)[i] = then[i] == SWIZZLE_SLOT_UNUSED  ? then[i] : first[then[i]];
      }
   }
}

static Dataflow *subscript_apply_swizzle(Dataflow *subscript, uint8_t *swizzle) {
   int member_count = 0;
   while (swizzle[member_count] != SWIZZLE_SLOT_UNUSED) member_count++;

   Dataflow *ret = glsl_dataflow_construct_const_int(swizzle[0]);
   for (int i=1; i<member_count; i++) {
      Dataflow *cond = glsl_dataflow_construct_binary_op(DATAFLOW_EQUAL, subscript, glsl_dataflow_construct_const_value(subscript->type, i));
      ret = glsl_dataflow_construct_ternary_op(DATAFLOW_CONDITIONAL,
                                               cond,
                                               glsl_dataflow_construct_const_int(swizzle[i]),
                                               ret);
   }

   return ret;
}

static void calculate_buffer_address(BasicBlock *ctx, Dataflow **address, Dataflow **offset, MEMBER_LAYOUT_T **layout, uint8_t **swizzle, Expr *expr)
{
   switch (expr->flavour)
   {
      case EXPR_INSTANCE:
         {
            Dataflow *base_scalar_value;

            // Assert that we are dealing with an interface block
            assert(expr->u.instance.symbol->flavour == SYMBOL_VAR_INSTANCE);
            assert(expr->u.instance.symbol->u.var_instance.block_info_valid);

            *layout = expr->u.instance.symbol->u.var_instance.block_info.layout;
            Symbol *block_symbol = expr->u.instance.symbol->u.var_instance.block_info.block_symbol;
            bool create_address = true;

            if (block_symbol) {
               base_scalar_value = glsl_basic_block_get_scalar_value(ctx, block_symbol, 0);
               if (block_symbol->u.interface_block.sq == STORAGE_SHARED) create_address = false;
            } else {
               // This instance is a uniform in the default uniform block.
               base_scalar_value = glsl_basic_block_get_scalar_value(ctx, expr->u.instance.symbol, 0);
               assert((*layout)->level_offset==0 && (*layout)->offset==0);
            }

            if (create_address)
               *address = glsl_dataflow_construct_address(base_scalar_value);
            else
               *address = glsl_dataflow_construct_reinterp(base_scalar_value, DF_INT);
            /* Blocks have offset of 0. If this is an anonymous this gives the offset of the member */
            *offset = glsl_dataflow_construct_const_int((*layout)->level_offset);
         }
         break;

      case EXPR_FIELD_SELECTOR:
         {
            // Recurse on aggregate.
            Dataflow *off;
            MEMBER_LAYOUT_T *aggregate_layout;
            calculate_buffer_address(ctx, address, &off, &aggregate_layout, swizzle, expr->u.field_selector.aggregate);

            if (aggregate_layout->flavour==MEMBER_STRUCT) {
               *layout = &aggregate_layout->u.struct_layout.member_layouts[expr->u.field_selector.field_no];
            } else {
               assert(aggregate_layout->flavour==MEMBER_BLOCK);
               *layout = &aggregate_layout->u.block_layout.member_layouts[expr->u.field_selector.field_no];
            }
            *offset = glsl_dataflow_construct_binary_op(DATAFLOW_ADD, off, glsl_dataflow_construct_const_int((*layout)->level_offset));
         }
         break;

      case EXPR_SWIZZLE:
         calculate_buffer_address(ctx, address, offset, layout, swizzle, expr->u.swizzle.aggregate);
         combine_address_swizzle(swizzle, expr->u.swizzle.swizzle_slots);
         break;

      case EXPR_SUBSCRIPT:
         {
            int member_count;
            Dataflow *subscript;
            Expr *aggregate = expr->u.subscript.aggregate;
            // Recurse on subscript.
            glsl_expr_calculate_dataflow(ctx, &subscript, expr->u.subscript.subscript);
            subscript = glsl_dataflow_construct_reinterp(subscript, DF_INT);

            if( aggregate->type->flavour==SYMBOL_ARRAY_TYPE &&
                aggregate->type->u.array_type.member_type->flavour==SYMBOL_BLOCK_TYPE )
            {
               /* In this case we don't recurse on aggregate. An array of blocks
                * is different from other arrays because the index determines
                * the binding point, rather than an offset. */
               Symbol     *block_symbol = aggregate->u.instance.symbol->u.var_instance.block_info.block_symbol;
               SymbolType *block_type   = aggregate->type->u.array_type.member_type;
               member_count = aggregate->type->u.array_type.member_count;

               assert(subscript->flavour == DATAFLOW_CONST);

               *layout = block_type->u.block_type.layout;
               *address = glsl_dataflow_construct_address(glsl_basic_block_get_scalar_value(ctx, block_symbol, subscript->u.constant.value));
               *offset = glsl_dataflow_construct_const_int(0);
            }
            else
            {
               int stride;
               Dataflow *off;
               MEMBER_LAYOUT_T *aggregate_layout;
               // Recurse on aggregate.
               calculate_buffer_address(ctx, address, &off, &aggregate_layout, swizzle, aggregate);

               if (*swizzle == NULL) {
                  switch(aggregate_layout->flavour)
                  {
                     case MEMBER_ARRAY:
                        {
                           assert(aggregate->type->flavour==SYMBOL_ARRAY_TYPE);

                           member_count = aggregate->type->u.array_type.member_count;
                           stride = aggregate_layout->u.array_layout.stride;

                           if (subscript->flavour == DATAFLOW_CONST && member_count > 0)
                              *layout = &aggregate_layout->u.array_layout.member_layouts[subscript->u.constant.value];
                           else  /* XXX Hackily use the first one as representative of the rest */
                              *layout = &aggregate_layout->u.array_layout.member_layouts[0];
                        }
                        break;
                     case MEMBER_PRIMITIVE_MATRIX:
                        {
                           bool column_major = aggregate_layout->u.matrix_layout.column_major;
                           int matrix_stride = aggregate_layout->u.matrix_layout.stride;

                           assert(expr->u.subscript.aggregate->type->flavour==SYMBOL_PRIMITIVE_TYPE);
                           assert(primitiveTypeFlags[expr->u.subscript.aggregate->type->u.primitive_type.index] & PRIM_MATRIX_TYPE);

                           member_count = glsl_prim_matrix_type_subscript_dimensions(aggregate->type->u.primitive_type.index, 0);

                           if(column_major) {
                              stride = matrix_stride;
                              *layout = construct_prim_vector_layout(4);
                           } else {
                              stride = 4;
                              *layout = construct_prim_vector_layout(matrix_stride);
                           }
                        }
                        break;
                     case MEMBER_PRIMITIVE_NONMATRIX:
                        {
                           *layout = construct_prim_vector_layout(0);

                           assert(aggregate->type->flavour==SYMBOL_PRIMITIVE_TYPE);
                           assert(primitiveTypeFlags[aggregate->type->u.primitive_type.index] & PRIM_VECTOR_TYPE);

                           member_count = aggregate->type->scalar_count;
                           stride = aggregate_layout->u.vector_layout.stride;
                        }
                        break;
                     default:
                        UNREACHABLE();
                  }
                  /* TODO: At the moment we don't bounds check SSBO array accesses. Should we? */
                  if (member_count > 0)
                     subscript = construct_bounds_check(subscript, member_count);
                  subscript = glsl_dataflow_construct_binary_op(DATAFLOW_MUL, subscript, glsl_dataflow_construct_const_int(stride));
                  *offset = glsl_dataflow_construct_binary_op(DATAFLOW_ADD, off, subscript);
               } else {
                  /* Subscripting a swizzle. Construct an if-then-else tree */
                  /* TODO: There are special cases where we could generate better code.
                   * Since noone will ever write code like this, I haven't done it */
                  assert(aggregate_layout->flavour == MEMBER_PRIMITIVE_NONMATRIX);
                  stride = aggregate_layout->u.vector_layout.stride;
                  Dataflow *swizzled_subscript = subscript_apply_swizzle(subscript, *swizzle);
                  *layout = construct_prim_vector_layout(0);
                  *offset = glsl_dataflow_construct_binary_op(DATAFLOW_MUL, swizzled_subscript, glsl_dataflow_construct_const_int(stride));
                  *swizzle = NULL;     /* The old swizzle has been taken care of already */
               }
            }
         }
         break;

      default:
         UNREACHABLE();
         return;
   }
}

static void buffer_load_expr_calculate_dataflow(BasicBlock *ctx, Dataflow **scalar_values, Expr *expr)
{
   Dataflow *address, *offset;
   MEMBER_LAYOUT_T *layout;

   uint8_t *swizzle = NULL;
   calculate_buffer_address(ctx, &address, &offset, &layout, &swizzle, expr);
   build_loads(scalar_values, address, offset, swizzle, layout, expr->type);
}

void buffer_store_expr_calculate_dataflow(BasicBlock *ctx, Expr *lvalue_expr, Dataflow **values)
{
   Dataflow *address, *offset;
   MEMBER_LAYOUT_T *layout;

   uint8_t *swizzle = NULL;
   calculate_buffer_address(ctx, &address, &offset, &layout, &swizzle, lvalue_expr);
   build_stores(ctx, address, offset, swizzle, values, layout, lvalue_expr->type);
}

/* Return whether this expr lives in a buffer (UBO or SSBO) */
static bool instance_in_buffer(Symbol *symbol)
{
   if (symbol->flavour == SYMBOL_TEMPORARY || symbol->flavour == SYMBOL_PARAM_INSTANCE)
      return false;

   assert(symbol->flavour==SYMBOL_VAR_INSTANCE);

   return symbol->u.var_instance.block_info_valid &&
          symbol->u.var_instance.block_info.block_symbol;
}

static bool is_uniform_array_instance(Symbol *symbol)
{
   if (symbol->flavour == SYMBOL_TEMPORARY || symbol->flavour == SYMBOL_PARAM_INSTANCE)
      return false;

   assert(symbol->flavour==SYMBOL_VAR_INSTANCE);

   return symbol->u.var_instance.block_info_valid &&
          symbol->type->flavour == SYMBOL_ARRAY_TYPE;
}

bool in_addressable_memory(Expr *expr)
{
   switch(expr->flavour)
   {
      case EXPR_INSTANCE:
         return instance_in_buffer(expr->u.instance.symbol) ||
                is_uniform_array_instance(expr->u.instance.symbol);

      case EXPR_SUBSCRIPT:
         return in_addressable_memory(expr->u.subscript.aggregate);

      case EXPR_FIELD_SELECTOR:
         return in_addressable_memory(expr->u.field_selector.aggregate);

      case EXPR_SWIZZLE:
         return in_addressable_memory(expr->u.swizzle.aggregate);

      default:
         return false;
   }
}

static inline void expr_calculate_dataflow_instance(BasicBlock *ctx, Dataflow **scalar_values, Expr *expr)
{
   Symbol *symbol = expr->u.instance.symbol;
   assert(symbol->flavour == SYMBOL_VAR_INSTANCE   ||
          symbol->flavour == SYMBOL_PARAM_INSTANCE ||
          symbol->flavour == SYMBOL_TEMPORARY);

   if(instance_in_buffer(symbol)) {
      buffer_load_expr_calculate_dataflow(ctx, scalar_values, expr);
   } else {
      memcpy(scalar_values, glsl_basic_block_get_scalar_values(ctx, symbol), sizeof(Dataflow*) * symbol->type->scalar_count);
   }
}

/* Exprs that are valid for subscripting: INSTANCE, FUNCTION_CALL and FIELD_SELECTOR */
static inline void expr_calculate_dataflow_subscript(BasicBlock* ctx, Dataflow **scalar_values, Expr *expr)
{
   Dataflow **aggregate_scalar_values;
   Dataflow *subscript_scalar;

   assert(expr->u.subscript.subscript->type->scalar_count == 1);

   glsl_expr_calculate_dataflow(ctx, &subscript_scalar, expr->u.subscript.subscript);
   assert(glsl_dataflow_is_integral_type(subscript_scalar));

   if (subscript_scalar->flavour == DATAFLOW_CONST && expr->u.subscript.aggregate->type->scalar_count > 0)
   {
      unsigned int scalar_index;
      scalar_index = subscript_scalar->u.constant.value * expr->type->scalar_count;

      /* If the index is out of range, clamp to 0 */
      if (scalar_index >= expr->u.subscript.aggregate->type->scalar_count) {
         scalar_index = 0;
      }

      // Recurse on aggregate.
      aggregate_scalar_values = alloc_dataflow(expr->u.subscript.aggregate->type->scalar_count);
      glsl_expr_calculate_dataflow(ctx, aggregate_scalar_values, expr->u.subscript.aggregate);
      memcpy(scalar_values, aggregate_scalar_values + scalar_index,
             expr->type->scalar_count * sizeof(Dataflow*));
      free_dataflow(aggregate_scalar_values);
   }
   else if (glsl_prim_is_prim_atomic_type(expr->type))
   {
      aggregate_scalar_values = alloc_dataflow(expr->u.subscript.aggregate->type->scalar_count);
      glsl_expr_calculate_dataflow(ctx, aggregate_scalar_values, expr->u.subscript.aggregate);
      Dataflow *addr = aggregate_scalar_values[0];
      Dataflow *four = glsl_dataflow_construct_const_value(subscript_scalar->type, 4);
      Dataflow *offset = glsl_dataflow_construct_binary_op(DATAFLOW_MUL, subscript_scalar, four);
      *scalar_values = glsl_dataflow_construct_binary_op(DATAFLOW_ADD, addr, glsl_dataflow_construct_reinterp(offset, DF_INT));
      free_dataflow(aggregate_scalar_values);
   }
   else if (in_addressable_memory(expr))
   {
      /* TODO: Here we redo all the dataflow calculation for subscript */
      buffer_load_expr_calculate_dataflow(ctx, scalar_values, expr);
   }
   else
   {
      aggregate_scalar_values = alloc_dataflow(expr->u.subscript.aggregate->type->scalar_count);
      glsl_expr_calculate_dataflow(ctx, aggregate_scalar_values, expr->u.subscript.aggregate);

      assert(expr->u.subscript.aggregate->type->flavour == SYMBOL_ARRAY_TYPE ||
             expr->u.subscript.aggregate->type->flavour == SYMBOL_PRIMITIVE_TYPE);

      int member_count = expr->u.subscript.aggregate->type->scalar_count /
                         expr->type->scalar_count;

      memcpy(scalar_values, aggregate_scalar_values, expr->type->scalar_count * sizeof(Dataflow*));
      for (int j=1; j<member_count; j++) {
         Dataflow *cond = glsl_dataflow_construct_binary_op(DATAFLOW_EQUAL, subscript_scalar, glsl_dataflow_construct_const_value(subscript_scalar->type, j));
         for (unsigned i=0; i<expr->type->scalar_count; i++) {
            scalar_values[i] = glsl_dataflow_construct_ternary_op(DATAFLOW_CONDITIONAL,
                                                                  cond,
                                                                  aggregate_scalar_values[j*expr->type->scalar_count + i],
                                                                  scalar_values[i]);
         }
      }

      free_dataflow(aggregate_scalar_values);
   }
}

static inline void expr_calculate_dataflow_sequence(BasicBlock *ctx, Dataflow **scalar_values, Expr *expr)
{
   Dataflow** all_these_scalar_values = alloc_dataflow(expr->u.sequence.all_these->type->scalar_count);
   glsl_expr_calculate_dataflow(ctx, all_these_scalar_values, expr->u.sequence.all_these);
   glsl_expr_calculate_dataflow(ctx, scalar_values, expr->u.sequence.then_this);
   free_dataflow(all_these_scalar_values);
}

static inline void expr_calculate_dataflow_array_length(BasicBlock *ctx, Dataflow **scalar_values, Expr *expr)
{
   Dataflow *address, *offset;
   MEMBER_LAYOUT_T *layout;

   uint8_t *swizzle = NULL;
   calculate_buffer_address(ctx, &address, &offset, &layout, &swizzle, expr->u.array_length.array);
   assert(address->flavour == DATAFLOW_ADDRESS);
   address = address->d.unary_op.operand;

   Dataflow *buf_size = glsl_dataflow_construct_buf_size(address);
   Dataflow *array_size = glsl_dataflow_construct_binary_op(DATAFLOW_SUB, buf_size, offset);
   scalar_values[0] = glsl_dataflow_construct_binary_op( DATAFLOW_DIV, array_size, glsl_dataflow_construct_const_int(layout->u.array_layout.stride));
}

static inline void expr_calculate_dataflow_prim_constructor_call(BasicBlock *ctx, Dataflow **scalar_values, Expr *expr)
{
   ExprChain *args = expr->u.prim_constructor_call.args;
   PrimitiveTypeIndex out_scalar_type_index;
   DataflowType df_out_type;

   assert(expr->type->flavour == SYMBOL_PRIMITIVE_TYPE);
   out_scalar_type_index = primitiveScalarTypeIndices[expr->type->u.primitive_type.index];
   df_out_type = glsl_prim_index_to_df_type(out_scalar_type_index);

   switch(expr->u.prim_constructor_call.flavour)
   {
      case PRIM_CONS_VECTOR_FROM_SCALAR:
      case PRIM_CONS_MATRIX_FROM_SCALAR:
         {
            Dataflow **arg_scalar_values = alloc_dataflow(args->first->expr->type->scalar_count);
            Dataflow *converted_scalar_value;

            // Recurse on first arg.
            glsl_expr_calculate_dataflow(ctx, arg_scalar_values, args->first->expr);

            // Convert first component of first arg.
            converted_scalar_value = glsl_dataflow_convert_type(arg_scalar_values[0], df_out_type);

            // Fill.
            if (expr->u.prim_constructor_call.flavour == PRIM_CONS_VECTOR_FROM_SCALAR)
            {
               int d = primitiveTypeSubscriptDimensions[expr->type->u.primitive_type.index];
               for (int i=0; i < d; i++) scalar_values[i] = converted_scalar_value;
            } else {              /* PRIM_CONS_MATRIX_FROM_SCALAR */
               int offset = 0;
               int cols = glsl_prim_matrix_type_subscript_dimensions(expr->type->u.primitive_type.index, 0);
               int rows = glsl_prim_matrix_type_subscript_dimensions(expr->type->u.primitive_type.index, 1);
               for (int i = 0; i < cols; i++) {
                  for (int j = 0; j < rows; j++) {
                     scalar_values[offset++] = (i == j) ? converted_scalar_value : glsl_dataflow_construct_const_float(0);
                  }
               }
            }

            free_dataflow(arg_scalar_values);
         }
         return;

      case PRIM_CONS_MATRIX_FROM_MATRIX:
         {
            int arg_cols, arg_rows, offset;
            int cols = glsl_prim_matrix_type_subscript_dimensions(expr->type->u.primitive_type.index, 0);
            int rows = glsl_prim_matrix_type_subscript_dimensions(expr->type->u.primitive_type.index, 1);
            Dataflow **arg_scalar_values = alloc_dataflow(args->first->expr->type->scalar_count);

            // Recurse on first arg.
            glsl_expr_calculate_dataflow(ctx, arg_scalar_values, args->first->expr);

            // Find first arg dimension.
            assert(args->first->expr->type->flavour == SYMBOL_PRIMITIVE_TYPE);
            arg_cols = glsl_prim_matrix_type_subscript_dimensions(args->first->expr->type->u.primitive_type.index, 0);
            arg_rows = glsl_prim_matrix_type_subscript_dimensions(args->first->expr->type->u.primitive_type.index, 1);

            // Fill.
            offset = 0;
            for (int i = 0; i < cols; i++) {
               for (int j = 0; j < rows; j++) {
                  scalar_values[offset++] = (i < arg_cols && j < arg_rows) ? arg_scalar_values[arg_rows * i + j]
                     : glsl_dataflow_construct_const_float((i == j) ? 1.0f : 0.0f);
               }
            }

            free_dataflow(arg_scalar_values);
         }
         return;

      case PRIM_CONS_FROM_COMPONENT_FLOW:
         {
            unsigned offset = 0;
            for (ExprChainNode *arg = args->first; arg; arg = arg->next)
            {
               Dataflow **arg_scalar_values = alloc_dataflow(arg->expr->type->scalar_count);
               unsigned arg_offset = 0;

               // Recurse on arg.
               glsl_expr_calculate_dataflow(ctx, arg_scalar_values, arg->expr);

               while (arg_offset < arg->expr->type->scalar_count && offset < expr->type->scalar_count)
               {
                  scalar_values[offset++] = glsl_dataflow_convert_type(arg_scalar_values[arg_offset++], df_out_type);
               }

               free_dataflow(arg_scalar_values);
            }
         }
         return;

      case PRIM_CONS_INVALID:
      default:
         UNREACHABLE();
         return;
   }
}

static inline void expr_calculate_dataflow_compound_constructor_call(BasicBlock *ctx, Dataflow **scalar_values, Expr *expr)
{
   ExprChainNode* arg;

   for (arg = expr->u.compound_constructor_call.args->first; arg; arg = arg->next)
   {
      Dataflow** arg_scalar_values = alloc_dataflow(arg->expr->type->scalar_count);

      // Recurse on arg.
      glsl_expr_calculate_dataflow(ctx, arg_scalar_values, arg->expr);

      // Copy out result.
      memcpy(scalar_values, arg_scalar_values, arg->expr->type->scalar_count * sizeof(Dataflow*));
      scalar_values += arg->expr->type->scalar_count;

      free_dataflow(arg_scalar_values);
   }
}

static inline void expr_calculate_dataflow_swizzle(BasicBlock *ctx, Dataflow **scalar_values, Expr *expr)
{
   Dataflow **aggregate_scalar_values = alloc_dataflow(expr->u.swizzle.aggregate->type->scalar_count);

   // Recurse on aggregate.
   glsl_expr_calculate_dataflow(ctx, aggregate_scalar_values, expr->u.swizzle.aggregate);

   for (int i = 0; i < MAX_SWIZZLE_FIELD_COUNT && expr->u.swizzle.swizzle_slots[i] != SWIZZLE_SLOT_UNUSED; i++)
   {
      assert(expr->u.swizzle.swizzle_slots[i] < expr->u.swizzle.aggregate->type->scalar_count);

      scalar_values[i] = aggregate_scalar_values[expr->u.swizzle.swizzle_slots[i]];
   }

   free_dataflow(aggregate_scalar_values);
}

static inline void expr_calculate_dataflow_field_selector(BasicBlock *ctx, Dataflow **scalar_values, Expr *expr)
{
   if (expr->u.field_selector.aggregate->type->flavour == SYMBOL_STRUCT_TYPE) {
      Dataflow **aggregate_scalar_values = alloc_dataflow(expr->u.field_selector.aggregate->type->scalar_count);
      Dataflow **field = aggregate_scalar_values;

      // Recurse on aggregate.
      glsl_expr_calculate_dataflow(ctx, aggregate_scalar_values, expr->u.field_selector.aggregate);

      for (int i = 0; i < expr->u.field_selector.field_no; i++)
         field += expr->u.field_selector.aggregate->type->u.struct_type.member[i].type->scalar_count;

      // Copy out field.
      memcpy(scalar_values, field, expr->type->scalar_count * sizeof(Dataflow*));

      free_dataflow(aggregate_scalar_values);
   } else {
      assert(in_addressable_memory(expr));
      buffer_load_expr_calculate_dataflow(ctx, scalar_values, expr);
   }
}

static void expr_calculate_dataflow_unary_op(BasicBlock* ctx, Dataflow** scalar_values, Expr* expr)
{
   DataflowFlavour dataflow_flavour;

   Expr      *operand = expr->u.unary_op.operand;
   Dataflow **operand_scalar_values = alloc_dataflow(operand->type->scalar_count);

   // Recurse.
   glsl_expr_calculate_dataflow(ctx, operand_scalar_values, operand);

   // Work out dataflow flavour.
   switch (expr->flavour)
   {
   // Logical.
   case EXPR_LOGICAL_NOT:
      dataflow_flavour = DATAFLOW_LOGICAL_NOT;
      break;
   case EXPR_BITWISE_NOT:
      dataflow_flavour = DATAFLOW_BITWISE_NOT;
      break;

   // Arithemetic.
   case EXPR_ARITH_NEGATE:
      dataflow_flavour = DATAFLOW_ARITH_NEGATE;
      break;

   default:
      UNREACHABLE();
      return;
   }

   // Create nodes.
   for (unsigned i = 0; i < expr->type->scalar_count; i++)
      scalar_values[i] = glsl_dataflow_construct_unary_op(dataflow_flavour, operand_scalar_values[i]);

   free_dataflow(operand_scalar_values);
}

static inline void expr_calculate_dataflow_binary_op_shift(BasicBlock* ctx, Dataflow** scalar_values, Expr* expr)
{
   Expr* left = expr->u.binary_op.left;
   Expr* right = expr->u.binary_op.right;
   ExprFlavour flavour = expr->flavour;
   PRIMITIVE_TYPE_FLAGS_T left_flags, right_flags;
   DataflowFlavour dataflow_flavour;
   Dataflow** left_scalar_values  = alloc_dataflow(left->type->scalar_count);
   Dataflow** right_scalar_values = alloc_dataflow(right->type->scalar_count);

   // Recurse.
   glsl_expr_calculate_dataflow(ctx, left_scalar_values,  left);
   glsl_expr_calculate_dataflow(ctx, right_scalar_values, right);

   /* Assert that these operands are compatible with shifting */
   left_flags  = primitiveTypeFlags[left->type->u.primitive_type.index];
   right_flags = primitiveTypeFlags[right->type->u.primitive_type.index];

   /* Must be int (scalar or vector) shifted by int scalar */
   assert( (left_flags & PRIM_INT_TYPE) || (left_flags & PRIM_UINT_TYPE) );
   assert( (right_flags & PRIM_INT_TYPE) || (right_flags & PRIM_UINT_TYPE) );
   assert( (left_flags & (PRIM_SCALAR_TYPE | PRIM_VECTOR_TYPE) ));
   assert( (right_flags & PRIM_SCALAR_TYPE) || (left_flags & PRIM_VECTOR_TYPE) );
   vcos_unused_in_release(left_flags);

   assert(flavour == EXPR_SHL || flavour == EXPR_SHR);
   if(flavour==EXPR_SHL)
      dataflow_flavour = DATAFLOW_SHL;
   else
      dataflow_flavour = DATAFLOW_SHR;

   for (unsigned i = 0; i < expr->type->scalar_count; i++)
   {
      if (right_flags & PRIM_SCALAR_TYPE) {
         scalar_values[i] = glsl_dataflow_construct_binary_op(
            dataflow_flavour,
            left_scalar_values[i],
            right_scalar_values[0]);
      } else {
         scalar_values[i] = glsl_dataflow_construct_binary_op(
            dataflow_flavour,
            left_scalar_values[i],
            right_scalar_values[i]);
      }
   }

   free_dataflow(right_scalar_values);
   free_dataflow(left_scalar_values);
}

static inline void construct_dataflow_matXxY_mul_matZxY_matXxZ(const int X, const int Y, const int Z, Dataflow** result_scalar_values, Dataflow** left_scalar_values, Dataflow** right_scalar_values)
{
   /* Linear algebraic matZxY * matXxZ. */
   for(int i = 0; i < X; i++)
   {
      for (int j = 0; j < Y; j++)
      {
         Dataflow *acc = NULL;

         // Fold in the rest.
         for (int k = 0; k < Z; k++)
         {
            Dataflow *mul = glsl_dataflow_construct_binary_op(
                                 DATAFLOW_MUL,
                                 left_scalar_values[k*Y + j],
                                 right_scalar_values[i*Z + k]);

            if (k == 0) acc = mul;
            else
               acc = glsl_dataflow_construct_binary_op(DATAFLOW_ADD, acc, mul);
         }

         result_scalar_values[j + i*Y] = acc;
      }
   }
}

static inline void expr_calculate_dataflow_binary_op_arithmetic(BasicBlock *ctx, Dataflow **scalar_values, Expr *expr)
{
   Expr *left = expr->u.binary_op.left;
   Expr *right = expr->u.binary_op.right;
   ExprFlavour flavour = expr->flavour;
   DataflowFlavour dataflow_flavour;
   Dataflow **left_scalar_values  = alloc_dataflow(left->type->scalar_count);
   Dataflow **right_scalar_values = alloc_dataflow(right->type->scalar_count);

   // Recurse.
   glsl_expr_calculate_dataflow(ctx, left_scalar_values, left);
   glsl_expr_calculate_dataflow(ctx, right_scalar_values, right);

   switch (flavour) {
   case EXPR_MUL:
      dataflow_flavour = DATAFLOW_MUL;
      break;
   case EXPR_DIV:
      dataflow_flavour = DATAFLOW_DIV;
      break;
   case EXPR_REM:
      dataflow_flavour = DATAFLOW_REM;
      break;
   case EXPR_ADD:
      dataflow_flavour = DATAFLOW_ADD;
      break;
   case EXPR_SUB:
      dataflow_flavour = DATAFLOW_SUB;
      break;
   default:
      UNREACHABLE();
      return;
   }

   int left_index = left->type->u.primitive_type.index;
   int right_index = right->type->u.primitive_type.index;
   PRIMITIVE_TYPE_FLAGS_T left_flags = primitiveTypeFlags[left_index];
   PRIMITIVE_TYPE_FLAGS_T right_flags = primitiveTypeFlags[right_index];

   // Infer type, and value if possible.
   // From spec (1.00 rev 14 p46), the two operands must be:
   // 1 - the same type (the type being integer scalar/vector, float scalar/vector/matrix),
   // 2 - or one can be a scalar float and the other a float vector or matrix,
   // 3 - or one can be a scalar integer and the other an integer vector,
   // 4 - or, for multiply, one can be a float vector and the other a float matrix with the same dimensional size.
   // All operations are component-wise except EXPR_MUL involving at least one matrix (cases 1 and 4).

   assert( ((left_flags & PRIM_FLOAT_TYPE) && (right_flags & PRIM_FLOAT_TYPE)) ||
           ((left_flags & PRIM_INT_TYPE)   && (right_flags & PRIM_INT_TYPE))   ||
           ((left_flags & PRIM_UINT_TYPE)   && (right_flags & PRIM_UINT_TYPE))   );

   // Case 1.
   if (left->type == right->type)
   {
      if (flavour == EXPR_MUL && (left_flags & PRIM_MATRIX_TYPE))
      {
         int d = primitiveTypeSubscriptDimensions[expr->type->u.primitive_type.index];
         // Linear algebraic mat * mat.
         construct_dataflow_matXxY_mul_matZxY_matXxZ(d,
                                                     d,
                                                     d,
                                                     scalar_values,
                                                     left_scalar_values,
                                                     right_scalar_values);
      }
      else
      {
         // Component-wise on same scalar type, same scalar count.
         for (unsigned i = 0; i < expr->type->scalar_count; i++)
         {
            assert(left_scalar_values[i]);
            assert(right_scalar_values[i]);

            scalar_values[i] = glsl_dataflow_construct_binary_op(
               dataflow_flavour,
               left_scalar_values[i],
               right_scalar_values[i]);
         }
      }
      free_dataflow(right_scalar_values);
      free_dataflow(left_scalar_values);
      return;
   }

   // Cases 2 and 3.
   if ( (left_flags & PRIM_SCALAR_TYPE) &&
        (right_flags & (PRIM_VECTOR_TYPE | PRIM_MATRIX_TYPE)))
   {
      // Component-wise on same scalar type, different scalar counts.
      for (unsigned i = 0; i < expr->type->scalar_count; i++)
      {
         scalar_values[i] = glsl_dataflow_construct_binary_op(
            dataflow_flavour,
            left_scalar_values[0],
            right_scalar_values[i]);
      }

      free_dataflow(right_scalar_values);
      free_dataflow(left_scalar_values);
      return;
   }
   else if ( (right_flags & PRIM_SCALAR_TYPE) &&
             (left_flags & (PRIM_VECTOR_TYPE | PRIM_MATRIX_TYPE)))
   {
      // Component-wise on same scalar type, different scalar counts.
      for (unsigned i = 0; i < expr->type->scalar_count; i++)
      {
         scalar_values[i] = glsl_dataflow_construct_binary_op(
            dataflow_flavour,
            left_scalar_values[i],
            right_scalar_values[0]);
      }

      free_dataflow(right_scalar_values);
      free_dataflow(left_scalar_values);
      return;
   }

   // Case 4.
   if (flavour == EXPR_MUL) {
      int X, Y, Z;
      if ( (left_flags & PRIM_MATRIX_TYPE) && (right_flags & PRIM_VECTOR_TYPE)
         && glsl_prim_matrix_type_subscript_dimensions(left_index, 0) == primitiveTypeSubscriptDimensions[left_index])
      {
         /* Linear algebraic mat * vec. */
         X = 1;
         Y = primitiveTypeSubscriptDimensions[expr->type->u.primitive_type.index];
         Z = glsl_prim_matrix_type_subscript_dimensions(left_index, 0);
      } else if ( (right_flags & PRIM_MATRIX_TYPE) && (left_flags & PRIM_VECTOR_TYPE)
         && primitiveTypeSubscriptDimensions[left_index] == glsl_prim_matrix_type_subscript_dimensions(right_index, 1))
      {
         /* Linear algebraic vec * mat. */
         X = primitiveTypeSubscriptDimensions[expr->type->u.primitive_type.index];
         Y = 1;
         Z = glsl_prim_matrix_type_subscript_dimensions(right_index, 1);
      }
      else if ( (right_flags & PRIM_MATRIX_TYPE) && (left_flags & PRIM_MATRIX_TYPE)
         && glsl_prim_matrix_type_subscript_dimensions(left_index, 0) == glsl_prim_matrix_type_subscript_dimensions(right_index, 1))
      {
         /* Linear algebraic mat * mat. */
         X = glsl_prim_matrix_type_subscript_dimensions(expr->type->u.primitive_type.index, 0);
         Y = glsl_prim_matrix_type_subscript_dimensions(expr->type->u.primitive_type.index, 1);
         Z = glsl_prim_matrix_type_subscript_dimensions(right_index, 1);
      }
      else
         UNREACHABLE();    /* AST should have checked that this is valid */

      construct_dataflow_matXxY_mul_matZxY_matXxZ(X,
                                                  Y,
                                                  Z,
                                                  scalar_values,
                                                  left_scalar_values,
                                                  right_scalar_values);

      free_dataflow(right_scalar_values);
      free_dataflow(left_scalar_values);
      return;
   }

   // All valid instances of expr should have been handled above.
   UNREACHABLE();
}

static void expr_calculate_dataflow_binary_op_bitwise(BasicBlock* ctx, Dataflow **scalar_values, Expr *expr)
{
   DataflowFlavour flavour;

   switch (expr->flavour) {
      case EXPR_BITWISE_AND: flavour = DATAFLOW_BITWISE_AND; break;
      case EXPR_BITWISE_OR:  flavour = DATAFLOW_BITWISE_OR;  break;
      case EXPR_BITWISE_XOR: flavour = DATAFLOW_BITWISE_XOR; break;
      default: UNREACHABLE(); break;
   }

   Expr *left  = expr->u.binary_op.left;
   Expr *right = expr->u.binary_op.right;

   Dataflow **left_scalar_values  = alloc_dataflow(left->type->scalar_count);
   Dataflow **right_scalar_values = alloc_dataflow(right->type->scalar_count);

   glsl_expr_calculate_dataflow(ctx, left_scalar_values, left);
   glsl_expr_calculate_dataflow(ctx, right_scalar_values, right);

   if (left->type->scalar_count == 1)
   {
      for (unsigned i=0; i<expr->type->scalar_count; i++) {
         scalar_values[i] = glsl_dataflow_construct_binary_op(flavour,
                                                              left_scalar_values[0],
                                                              right_scalar_values[i]);
      }
   }
   else if (right->type->scalar_count == 1)
   {
      for (unsigned i=0; i<expr->type->scalar_count; i++) {
         scalar_values[i] = glsl_dataflow_construct_binary_op(flavour,
                                                              left_scalar_values[i],
                                                              right_scalar_values[0]);
      }
   }
   else
   {
      for (unsigned i=0; i<expr->type->scalar_count; i++) {
         scalar_values[i] = glsl_dataflow_construct_binary_op(flavour,
                                                              left_scalar_values[i],
                                                              right_scalar_values[i]);
      }
   }

   free_dataflow(right_scalar_values);
   free_dataflow(left_scalar_values);
}

// Not inlined as it's common.
static void expr_calculate_dataflow_binary_op_common(BasicBlock* ctx, Dataflow** scalar_values, Expr* expr)
{
   Dataflow *left_scalar_value;
   Dataflow *right_scalar_value;
   DataflowFlavour flavour;

   Expr *left = expr->u.binary_op.left;
   Expr *right = expr->u.binary_op.right;

   // This code assumes we do not act on vectors.
   assert(left->type->scalar_count  == 1);
   assert(right->type->scalar_count == 1);

   // Recurse.
   glsl_expr_calculate_dataflow(ctx, &left_scalar_value, left);
   glsl_expr_calculate_dataflow(ctx, &right_scalar_value, right);

   // Work out dataflow flavour.
   switch (expr->flavour) {
   // Logical.
   case EXPR_LOGICAL_XOR:        flavour = DATAFLOW_LOGICAL_XOR;        break;
   // Relational.
   case EXPR_LESS_THAN:          flavour = DATAFLOW_LESS_THAN;          break;
   case EXPR_LESS_THAN_EQUAL:    flavour = DATAFLOW_LESS_THAN_EQUAL;    break;
   case EXPR_GREATER_THAN:       flavour = DATAFLOW_GREATER_THAN;       break;
   case EXPR_GREATER_THAN_EQUAL: flavour = DATAFLOW_GREATER_THAN_EQUAL; break;

   default:
      UNREACHABLE();
      return;
   }

   // Create node.
   scalar_values[0] = glsl_dataflow_construct_binary_op(flavour,
                                                        left_scalar_value,
                                                        right_scalar_value);
}

static inline void expr_calculate_dataflow_binary_op_equality(BasicBlock* ctx, Dataflow** scalar_values, Expr* expr)
{
   Expr* left = expr->u.binary_op.left;
   Expr* right = expr->u.binary_op.right;
   Dataflow** left_scalar_values  = alloc_dataflow(left->type->scalar_count);
   Dataflow** right_scalar_values = alloc_dataflow(right->type->scalar_count);
   Dataflow* acc = NULL;
   unsigned int i;

   // Recurse.
   glsl_expr_calculate_dataflow(ctx, left_scalar_values,  left);
   glsl_expr_calculate_dataflow(ctx, right_scalar_values, right);

   assert(left->type->scalar_count == right->type->scalar_count);

   for (i = 0; i < left->type->scalar_count; i++) {
      DataflowFlavour flavour, acc_flavour;
      Dataflow* cmp;
      switch (expr->flavour) {
      case EXPR_EQUAL:
         flavour = DATAFLOW_EQUAL;
         acc_flavour = DATAFLOW_LOGICAL_AND;
         break;
      case EXPR_NOT_EQUAL:
         flavour = DATAFLOW_NOT_EQUAL;
         acc_flavour = DATAFLOW_LOGICAL_OR;
         break;
      default:
         UNREACHABLE();
      }
      cmp = glsl_dataflow_construct_binary_op(flavour, left_scalar_values[i], right_scalar_values[i]);
      if (acc)
         acc = glsl_dataflow_construct_binary_op(acc_flavour, acc, cmp);
      else
         acc = cmp;
   }

   scalar_values[0] = acc;

   free_dataflow(right_scalar_values);
   free_dataflow(left_scalar_values);
}

void glsl_expr_calculate_function_call_args(BasicBlock *ctx, Symbol *function, ExprChain *args)
{
   int i;
   ExprChainNode *node;

   // Note we do this first into a temporary buffer and then copy the temporaries into the formal parameter storage
   // so that we work correctly for nested invocations like min(a, min(b, c))
   Dataflow ***temps = glsl_stack_malloc(sizeof(Dataflow**), function->type->u.function_type.param_count);

   for (i = 0, node = args->first; node; i++, node = node->next) {
      Symbol *formal = function->type->u.function_type.params[i];
      temps[i] = alloc_dataflow(formal->type->scalar_count);
      if (formal->u.param_instance.param_qual == PARAM_QUAL_IN || formal->u.param_instance.param_qual == PARAM_QUAL_INOUT)
         glsl_expr_calculate_dataflow(ctx, temps[i], node->expr);
   }

   for (i = 0, node = args->first; node; i++, node = node->next) {
      Symbol *formal = function->type->u.function_type.params[i];
      if (formal->u.param_instance.param_qual == PARAM_QUAL_IN || formal->u.param_instance.param_qual == PARAM_QUAL_INOUT)
         glsl_basic_block_set_scalar_values(ctx, formal, temps[i]);
   }

   for (i = function->type->u.function_type.param_count - 1; i >= 0; i--)
      free_dataflow(temps[i]);
   glsl_stack_free(temps);
}

// only trivial functions are allowed in expressions
// (no side-effects, no outs, no control-flow, no recursion, no missing return)
static inline void expr_calculate_dataflow_function_call(BasicBlock* ctx, Dataflow** scalar_values, Expr* expr)
{
   Symbol *function = expr->u.function_call.function;
   const NStmt* function_def = function->u.function_instance.function_norm_def;
   const NStmt* return_stmt = function_def->u.function_def.body->head->v;
   assert(return_stmt->flavour == NSTMT_RETURN_EXPR);
   glsl_expr_calculate_function_call_args(ctx, expr->u.function_call.function, expr->u.function_call.args);
   glsl_expr_calculate_dataflow(ctx, scalar_values, return_stmt->u.return_expr.expr);
}

//
// Dataflow calculation.
//

// Writes dataflow graph pointers for expr to scalar_values.
// scalar_values is array of Dataflow* with expr->type->scalar_count members.
void glsl_expr_calculate_dataflow(BasicBlock *ctx, Dataflow **scalar_values, Expr *expr)
{
   if (expr->compile_time_value) {
      // Constant expression.
      expr_calculate_dataflow_compile_time_value(scalar_values, expr->compile_time_value, expr->type);
   } else {
      // Non-constant expression.
      switch (expr->flavour) {
      case EXPR_FUNCTION_CALL:
         expr_calculate_dataflow_function_call(ctx, scalar_values, expr);
         break;

      case EXPR_SWIZZLE:
         expr_calculate_dataflow_swizzle(ctx, scalar_values, expr);
         break;

      case EXPR_SEQUENCE:
         expr_calculate_dataflow_sequence(ctx, scalar_values, expr);
         break;

      case EXPR_ARRAY_LENGTH:
         expr_calculate_dataflow_array_length(ctx, scalar_values, expr);
         break;

      case EXPR_INSTANCE:
         expr_calculate_dataflow_instance(ctx, scalar_values, expr);
         break;

      case EXPR_SUBSCRIPT:
         expr_calculate_dataflow_subscript(ctx, scalar_values, expr);
         break;

      case EXPR_PRIM_CONSTRUCTOR_CALL:
         expr_calculate_dataflow_prim_constructor_call(ctx, scalar_values, expr);
         break;

      case EXPR_COMPOUND_CONSTRUCTOR_CALL:
         expr_calculate_dataflow_compound_constructor_call(ctx, scalar_values, expr);
         return;

      case EXPR_FIELD_SELECTOR:
         expr_calculate_dataflow_field_selector(ctx, scalar_values, expr);
         break;

      case EXPR_ARITH_NEGATE:
      case EXPR_LOGICAL_NOT:
      case EXPR_BITWISE_NOT:
         expr_calculate_dataflow_unary_op(ctx, scalar_values, expr);
         break;

      case EXPR_MUL:
      case EXPR_DIV:
      case EXPR_REM:
      case EXPR_ADD:
      case EXPR_SUB:
         expr_calculate_dataflow_binary_op_arithmetic(ctx, scalar_values, expr);
         break;

      case EXPR_LESS_THAN:
      case EXPR_LESS_THAN_EQUAL:
      case EXPR_GREATER_THAN:
      case EXPR_GREATER_THAN_EQUAL:
         expr_calculate_dataflow_binary_op_common(ctx, scalar_values, expr);
         break;

      case EXPR_EQUAL:
      case EXPR_NOT_EQUAL:
         expr_calculate_dataflow_binary_op_equality(ctx, scalar_values, expr);
         break;

      case EXPR_LOGICAL_XOR:
         expr_calculate_dataflow_binary_op_common(ctx, scalar_values, expr);
         break;

      case EXPR_BITWISE_AND:
      case EXPR_BITWISE_OR:
      case EXPR_BITWISE_XOR:
         expr_calculate_dataflow_binary_op_bitwise(ctx, scalar_values, expr);
         break;

      case EXPR_SHL:
      case EXPR_SHR:
         expr_calculate_dataflow_binary_op_shift(ctx, scalar_values, expr);
         break;

      case EXPR_INTRINSIC:
         glsl_intrinsic_ir_calculate_dataflow(ctx, scalar_values, expr);
         break;

      // Expressions with side-effects or control-flow must be handled at statement level
      case EXPR_ASSIGN:
      case EXPR_POST_INC:
      case EXPR_POST_DEC:
      case EXPR_PRE_INC:
      case EXPR_PRE_DEC:
      case EXPR_LOGICAL_AND:
      case EXPR_LOGICAL_OR:
      case EXPR_CONDITIONAL:
         UNREACHABLE();
         break;

      // There should not be any of these for which expr->compile_time_value is NULL.
      case EXPR_VALUE:
      default:
         UNREACHABLE();
         break;
      }
   }
}
