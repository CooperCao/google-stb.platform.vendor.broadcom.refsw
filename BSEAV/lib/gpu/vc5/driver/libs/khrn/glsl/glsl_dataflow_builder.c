/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include <stdlib.h>

#include "glsl_nast.h"
#include "glsl_common.h"
#include "glsl_globals.h"
#include "glsl_dataflow.h"
#include "glsl_dataflow_builder.h"
#include "glsl_basic_block.h"
#include "glsl_intrinsic_ir_builder.h"
#include "glsl_stackmem.h"
#include "glsl_ast_visitor.h"
#include "glsl_errors.h"
#include "glsl_fastmem.h"
#include "glsl_const_operators.h"
#include "glsl_primitive_types.auto.h"
#include "glsl_stdlib.auto.h"
#include "glsl_symbol_table.h"
#include "glsl_constants.h"

#include "libs/util/gfx_util/gfx_util.h"
#include "libs/core/v3d/v3d_align.h"

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

struct buf_ref {
   Dataflow  *address;
   Dataflow  *offset;
   MemLayout *layout;
   uint8_t   *swizzle;
   StorageQualifier sq;
};

/* TODO: This inelegantly passes the whole buf_ref down, but also overrides the offset and
 *       layout fields. Ignore the ones in the buf_ref and use those passed in. */
typedef void (*LoadStoreFunc)(BasicBlock *b, const struct buf_ref *buf, Dataflow *offset,
                               Dataflow **scalars, MemLayout *layout, SymbolType *type);

static void do_store(BasicBlock *b, Dataflow *address, Dataflow *offset, uint8_t *swizzle, Dataflow **values, unsigned count, int stride) {
   uint8_t no_swizzle[4] = { 0, 1, 2, 3 };
   if (swizzle == NULL) swizzle = no_swizzle;

   Dataflow *v[4] = { NULL, };
   assert(count <= 4);
   for (unsigned i=0; i<count; i++) v[swizzle[i]] = values[i];

   uint32_t mask = 0;
   for (int i = 0; i < 4; i++) if (v[i] != NULL) mask |= (1 << i);

   while (mask != 0) {
#if !V3D_VER_AT_LEAST(4,1,34,0)
      int start = gfx_lsb(mask);

      /* Store as vectors where possible (continuous and sufficiently aligned) */
      int count = 1;
      if (stride == 4 && (start & 1) == 0)
         while (start + count < 4 && v[start+count] != NULL) count++;

      uint32_t store_mask = gfx_mask(count) << start;
#else
      int start;
      if (mask == 0xF) start = 0;
      else if ((mask & 0x9) == 9) {
         start = 3;
         while (v[start-1] != NULL) start--;
         assert(start > 0);
      } else start = gfx_lsb(mask);

      /* Store as vectors where continuous */
      int count = 1;
      if (stride == 4)
         while (count < 4 && v[(start + count) & 3] != NULL) count++;

      unsigned wrap_amount = (start + count >= 4) ? start + count - 4 : 0;
      uint32_t store_mask = gfx_mask(count - wrap_amount) << start | gfx_mask(wrap_amount);
#endif

      Dataflow *vec[4] = { NULL, };
      for (int i = 0; i<count; i++) vec[i] = v[(start + i) & 3];

      Dataflow *o = glsl_dataflow_construct_binary_op(DATAFLOW_ADD, offset, glsl_dataflow_construct_const_uint(stride * start));
      Dataflow *addr = glsl_dataflow_construct_binary_op(DATAFLOW_ADD, address, o);
      Dataflow *vs = glsl_dataflow_construct_vec4(vec[0], vec[1], vec[2], vec[3]);
      Dataflow *s = glsl_dataflow_construct_atomic(DATAFLOW_ADDRESS_STORE, DF_VOID, addr, vs, NULL, b->memory_head);
      b->memory_head = s;

      mask &= ~store_mask;
   }
}

static void build_stores(BasicBlock *b, const struct buf_ref *buf, Dataflow *offset,
                         Dataflow **values, MemLayout *layout, SymbolType *type)
{
   assert(layout->flavour == LAYOUT_PRIM_MATRIX || layout->flavour == LAYOUT_PRIM_NONMATRIX);

   if (layout->flavour == LAYOUT_PRIM_MATRIX) {
      bool row_major    = layout->u.prim_matrix_layout.row_major;
      int matrix_stride = layout->u.prim_matrix_layout.stride;

      assert(type->flavour==SYMBOL_PRIMITIVE_TYPE);
      assert(primitiveTypeFlags[type->u.primitive_type.index] & PRIM_MATRIX_TYPE);

      unsigned columns = glsl_prim_matrix_type_subscript_dimensions(type->u.primitive_type.index, 0);
      unsigned rows    = glsl_prim_matrix_type_subscript_dimensions(type->u.primitive_type.index, 1);

      assert(type->scalar_count==columns*rows);

      /* Consider the matrix as n_vecs vectors, each vec_size long */
      unsigned int n_vecs, vec_size;
      if(row_major) {
         n_vecs = rows;       /* Row vectors */
         vec_size = columns;
      } else {
         n_vecs = columns;    /* Column vectors */
         vec_size = rows;
      }

      Dataflow *out_vals[4][4];
      for (unsigned i=0; i<n_vecs; i++) {
         for (unsigned j=0; j<vec_size; j++) out_vals[i][j] = row_major ? values[j*n_vecs + i] : values[i*vec_size + j];
      }

      for(unsigned i=0; i<n_vecs; i++) {
         Dataflow *vec_offset = glsl_dataflow_construct_binary_op(DATAFLOW_ADD, offset, glsl_dataflow_construct_const_uint(i*matrix_stride));
         do_store(b, buf->address, vec_offset, buf->swizzle, &out_vals[i][0], vec_size, 4);
      }
   } else {
      assert(type->flavour==SYMBOL_PRIMITIVE_TYPE);
      Dataflow *v[4] = { NULL, };
      for (unsigned i=0; i<type->scalar_count; i++) {
         if (primitiveTypeFlags[type->u.primitive_type.index] & PRIM_BOOL_TYPE)
            v[i] = glsl_dataflow_convert_type(values[i], DF_INT);
         else
            v[i] = values[i];
      }
      int stride = layout->u.prim_nonmatrix_layout.stride;
      do_store(b, buf->address, offset, buf->swizzle, v, type->scalar_count, stride);
   }
}

static void build_loads(BasicBlock *b, const struct buf_ref *buf, Dataflow *offset,
                        Dataflow **scalar_offsets, MemLayout *layout, SymbolType *type)
{
   /* TODO: We only get into buffer addressing code from a field selector (which can't have swizzle)
    * or a dynamic index (which would fold the swizzle into the address calculation), so this can't
    * happen. It would actually be nice if it could, because then things like foo = vec_load.w wouldn't
    * be so awful */
   assert(buf->swizzle == NULL);
   (void)b;

   switch(layout->flavour)
   {
      case LAYOUT_PRIM_MATRIX:
         {
            bool row_major    = layout->u.prim_matrix_layout.row_major;
            int matrix_stride = layout->u.prim_matrix_layout.stride;
            unsigned int n_vecs, vec_size;
            Dataflow *matrix_scalars[4][4];

            assert(type->flavour==SYMBOL_PRIMITIVE_TYPE);
            assert(primitiveTypeFlags[type->u.primitive_type.index] & PRIM_MATRIX_TYPE);

            unsigned columns = glsl_prim_matrix_type_subscript_dimensions(type->u.primitive_type.index, 0);
            unsigned rows    = glsl_prim_matrix_type_subscript_dimensions(type->u.primitive_type.index, 1);

            assert(type->scalar_count==columns*rows);

            /* Consider the matrix as n_vecs vectors, each vec_size long */
            if(row_major) {
               n_vecs = rows;       /* Row vectors */
               vec_size = columns;
            } else {
               n_vecs = columns;    /* Column vectors */
               vec_size = rows;
            }

            for(unsigned i=0; i<n_vecs; i++)
            {
               Dataflow *vec_offset = glsl_dataflow_construct_binary_op(DATAFLOW_ADD, offset, glsl_dataflow_construct_const_uint(i*matrix_stride));
               Dataflow *vec_address = glsl_dataflow_construct_binary_op(DATAFLOW_ADD, buf->address, vec_offset);
               Dataflow *vec = glsl_dataflow_construct_vector_load(vec_address);

               for(unsigned j=0; j<vec_size; j++)
                  matrix_scalars[i][j] = glsl_dataflow_construct_get_vec4_component(j, vec, df_scalar_type(type, 0));
            }

            for (unsigned i=0; i<n_vecs; i++) {
               for (unsigned j=0; j<vec_size; j++) {
                  int scalar_idx;
                  /* Scalar offsets is column major, so row major transposes */
                  if (row_major) scalar_idx = j*rows + i;
                  else           scalar_idx = i*rows + j;

                  scalar_offsets[scalar_idx] = matrix_scalars[i][j];
               }
            }
         }
         break;
      case LAYOUT_PRIM_NONMATRIX:
         {
            assert(type->flavour==SYMBOL_PRIMITIVE_TYPE);

            int stride = layout->u.prim_nonmatrix_layout.stride;
            if (stride != 4) {
               for(unsigned i=0; i<type->scalar_count; i++) {
                  Dataflow *o = glsl_dataflow_construct_binary_op(DATAFLOW_ADD, offset, glsl_dataflow_construct_const_uint(i*stride));
                  Dataflow *address = glsl_dataflow_construct_binary_op(DATAFLOW_ADD, buf->address, o);
                  Dataflow *data    = glsl_dataflow_construct_vector_load(address);
                  scalar_offsets[i] = glsl_dataflow_construct_get_vec4_component(0, data, df_scalar_type(type, 0));
               }
            } else {
               Dataflow *vec_addr = glsl_dataflow_construct_binary_op(DATAFLOW_ADD, buf->address, offset);
               Dataflow *vec = glsl_dataflow_construct_vector_load(vec_addr);
               for (unsigned i=0; i<type->scalar_count; i++) {
                  scalar_offsets[i] = glsl_dataflow_construct_get_vec4_component(i, vec, df_scalar_type(type, 0));
               }
            }
         }
         break;

   default:
      unreachable();
   }
}

static void lower_loads_stores(BasicBlock *b, const struct buf_ref *buf, Dataflow *offset,
                               Dataflow **scalar_offsets, MemLayout *layout, SymbolType *type,
                               LoadStoreFunc load_or_store)
{
   switch (layout->flavour) {
      case LAYOUT_STRUCT:
      {
         int member_scalar_offset = 0;

         assert(type->flavour==SYMBOL_STRUCT_TYPE);

         for(unsigned i=0; i<layout->u.struct_layout.member_count; i++)
         {
            int member_addr_offset = layout->u.struct_layout.member_offsets[i];
            Dataflow *member_offset = glsl_dataflow_construct_binary_op(DATAFLOW_ADD, offset, glsl_dataflow_construct_const_uint(member_addr_offset));

            lower_loads_stores(b, buf, member_offset, scalar_offsets + member_scalar_offset, &layout->u.struct_layout.member_layouts[i], type->u.struct_type.member[i].type, load_or_store);
            member_scalar_offset += type->u.struct_type.member[i].type->scalar_count;
         }
         break;
      }
      case LAYOUT_ARRAY:
      {
         int array_stride   = layout->u.array_layout.stride;
         int member_scalars = type->u.array_type.member_type->scalar_count;
         int member_offset  = 0;

         assert(type->flavour==SYMBOL_ARRAY_TYPE);

         for(unsigned i=0; i<layout->u.array_layout.member_count; i++)
         {
            Dataflow *new_off = glsl_dataflow_construct_binary_op(DATAFLOW_ADD, offset, glsl_dataflow_construct_const_uint(member_offset));

            lower_loads_stores(b, buf, new_off, scalar_offsets + i * member_scalars, layout->u.array_layout.member_layout, type->u.array_type.member_type, load_or_store);
            member_offset += array_stride;
         }
         break;
      }
      case LAYOUT_PRIM_MATRIX:
      case LAYOUT_PRIM_NONMATRIX:
         load_or_store(b, buf, offset, scalar_offsets, layout, type);
         break;

   default:
      unreachable();
   }
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
         (*swizzle)[i] = then[i] == SWIZZLE_SLOT_UNUSED ? then[i] : first[then[i]];
      }
   }
}

static Dataflow *subscript_apply_swizzle(Dataflow *subscript, uint8_t *swizzle) {
   int member_count = 0;
   while (swizzle[member_count] != SWIZZLE_SLOT_UNUSED) member_count++;

   Dataflow *ret = glsl_dataflow_construct_const_uint(swizzle[0]);
   for (int i=1; i<member_count; i++) {
      Dataflow *cond = glsl_dataflow_construct_binary_op(DATAFLOW_EQUAL, subscript, glsl_dataflow_construct_const_value(subscript->type, i));
      ret = glsl_dataflow_construct_ternary_op(DATAFLOW_CONDITIONAL,
                                               cond,
                                               glsl_dataflow_construct_const_uint(swizzle[i]),
                                               ret);
   }

   return ret;
}

static Dataflow *clamp_subscript_index(Dataflow *subscript, unsigned member_count)
{
   Dataflow *index = glsl_dataflow_construct_reinterp(subscript, DF_UINT);
   Dataflow *max = glsl_dataflow_construct_const_value(DF_UINT, member_count - 1);
   return glsl_dataflow_construct_binary_op(DATAFLOW_MIN, index, max);
}

static void calculate_buffer_address(BasicBlock *b, struct buf_ref *buf, Expr *expr)
{
   switch (expr->flavour)
   {
      case EXPR_INSTANCE:
         {
            const Symbol *s = expr->u.instance.symbol;

            // Assert that we are dealing with an interface block
            assert(s->flavour == SYMBOL_VAR_INSTANCE);
            assert(s->u.var_instance.block_info_valid);
            unsigned member_offset = 0;

            buf->layout = s->u.var_instance.block_info.layout;
            buf->sq = s->u.var_instance.storage_qual;

            Symbol *block_symbol = s->u.var_instance.block_info.block_symbol;
            Dataflow *base_scalar_value;
            if (block_symbol) {
               base_scalar_value = glsl_basic_block_get_scalar_value(b, block_symbol, 0);
               if (s->u.var_instance.block_info.field_no != -1)
                  member_offset = block_symbol->u.interface_block.block_data_type->u.block_type.layout->u.struct_layout.member_offsets[s->u.var_instance.block_info.field_no];
            } else {
               // This instance is in a default block.
               base_scalar_value = glsl_basic_block_get_scalar_value(b, expr->u.instance.symbol, 0);
            }

            if (buf->sq != STORAGE_SHARED)
               buf->address = glsl_dataflow_construct_address(base_scalar_value);
            else
               buf->address = glsl_dataflow_construct_reinterp(base_scalar_value, DF_UINT);
            /* Blocks have offset of 0. If this is an anonymous this gives the offset of the member */
            buf->offset = glsl_dataflow_construct_const_uint(member_offset);
         }
         break;

      case EXPR_FIELD_SELECTOR:
         // Recurse on aggregate.
         calculate_buffer_address(b, buf, expr->u.field_selector.aggregate);

         int offset  =  buf->layout->u.struct_layout.member_offsets[expr->u.field_selector.field_no];
         buf->layout = &buf->layout->u.struct_layout.member_layouts[expr->u.field_selector.field_no];
         buf->offset = glsl_dataflow_construct_binary_op(DATAFLOW_ADD, buf->offset,
                                    glsl_dataflow_construct_const_uint(offset));
         break;

      case EXPR_SWIZZLE:
         calculate_buffer_address(b, buf, expr->u.swizzle.aggregate);
         combine_address_swizzle(&buf->swizzle, expr->u.swizzle.swizzle_slots);
         break;

      case EXPR_SUBSCRIPT:
         {
            // Recurse on subscript.
            Dataflow *subscript;
            glsl_expr_calculate_dataflow(b, &subscript, expr->u.subscript.subscript);
            subscript = glsl_dataflow_construct_reinterp(subscript, DF_UINT);

            Expr *aggregate = expr->u.subscript.aggregate;
            if( aggregate->type->flavour==SYMBOL_ARRAY_TYPE &&
                aggregate->type->u.array_type.member_type->flavour==SYMBOL_BLOCK_TYPE )
            {
               /* In this case we don't recurse on aggregate. An array of blocks
                * is different from other arrays because the index determines
                * the binding point, rather than an offset. */
               Symbol     *block_symbol = aggregate->u.instance.symbol->u.var_instance.block_info.block_symbol;
               SymbolType *block_type   = aggregate->type->u.array_type.member_type;
               int member_count = aggregate->type->u.array_type.member_count;

               if (subscript->flavour == DATAFLOW_CONST) {
                  /* Short-cut the calculation for constant subscripts */
                  uint32_t i = subscript->u.constant.value;
                  buf->address = glsl_dataflow_construct_address(glsl_basic_block_get_scalar_value(b, block_symbol, i));
               } else {
                  /* Find the correct block using an if-tree. TODO: Should we load the address from memory? */
                  Dataflow **aggregate_scalar_values = alloc_dataflow(member_count);
                  for (int i=0; i<member_count; i++) aggregate_scalar_values[i] = glsl_dataflow_construct_address(glsl_basic_block_get_scalar_value(b, block_symbol, i));

                  buf->address = aggregate_scalar_values[0];
                  for (int j=1; j<member_count; j++) {
                     Dataflow *cond = glsl_dataflow_construct_binary_op(DATAFLOW_EQUAL, subscript, glsl_dataflow_construct_const_uint(j));
                     buf->address = glsl_dataflow_construct_ternary_op(DATAFLOW_CONDITIONAL,
                                                                       cond,
                                                                       aggregate_scalar_values[j],
                                                                       buf->address);
                  }

                  free_dataflow(aggregate_scalar_values);
               }

               buf->layout = block_type->u.block_type.layout;
               buf->offset = glsl_dataflow_construct_const_uint(0);
               buf->sq     = block_symbol->u.interface_block.sq;
            }
            else
            {
               // Recurse on aggregate.
               calculate_buffer_address(b, buf, aggregate);

               if (buf->swizzle == NULL) {
                  unsigned stride, member_count;
                  switch(buf->layout->flavour)
                  {
                     case LAYOUT_ARRAY:
                        assert(aggregate->type->flavour==SYMBOL_ARRAY_TYPE);

                        member_count = aggregate->type->u.array_type.member_count;
                        stride = buf->layout->u.array_layout.stride;

                        buf->layout = buf->layout->u.array_layout.member_layout;
                        break;
                     case LAYOUT_PRIM_MATRIX:
                     {
                        bool row_major    = buf->layout->u.prim_matrix_layout.row_major;
                        int matrix_stride = buf->layout->u.prim_matrix_layout.stride;

                        assert(expr->u.subscript.aggregate->type->flavour==SYMBOL_PRIMITIVE_TYPE);
                        assert(primitiveTypeFlags[expr->u.subscript.aggregate->type->u.primitive_type.index] & PRIM_MATRIX_TYPE);

                        member_count = glsl_prim_matrix_type_subscript_dimensions(aggregate->type->u.primitive_type.index, 0);

                        if(row_major) {
                           stride = 4;
                           buf->layout = glsl_mem_prim_nonmatrix_layout(matrix_stride);
                        } else {
                           stride = matrix_stride;
                           buf->layout = glsl_mem_prim_nonmatrix_layout(4);
                        }
                        break;
                     }
                     case LAYOUT_PRIM_NONMATRIX:
                        assert(aggregate->type->flavour==SYMBOL_PRIMITIVE_TYPE);
                        assert(primitiveTypeFlags[aggregate->type->u.primitive_type.index] & PRIM_VECTOR_TYPE);

                        member_count = aggregate->type->scalar_count;
                        stride = buf->layout->u.prim_nonmatrix_layout.stride;
                        buf->layout = glsl_mem_prim_nonmatrix_layout(0);
                        break;
                     default:
                        unreachable();
                  }

                  /* Do the bounds check here. */
                  unsigned max_member_count;
                  if (member_count > 0) {
                     max_member_count = member_count;
                     subscript = clamp_subscript_index(subscript, member_count);
                  } else {
                     assert(buf->sq == STORAGE_BUFFER);
                     assert(buf->address->flavour == DATAFLOW_ADDRESS);

                     // There can only be one dynamically sized array in this buffer.
                     Dataflow *last = glsl_dataflow_construct_buf_array_length(buf->address->d.unary_op.operand, 1 /* get length-1 */);
                     subscript = glsl_dataflow_construct_binary_op(DATAFLOW_MIN, subscript, last);
                     max_member_count = GLSL_MAX_SHADER_STORAGE_BLOCK_SIZE / stride;
                  }

                  /* Use cheaper 24-bit multiply if we know this is possible. */
                  DataflowFlavour mul = DATAFLOW_MUL;
                  if (max_member_count <= (1 << 24) && stride < (1 << 24))
                     mul = DATAFLOW_MUL24;

                  /* Compute buffer offset. */
                  Dataflow *df_stride = glsl_dataflow_construct_const_uint(stride);
                  Dataflow *offset = glsl_dataflow_construct_binary_op(mul, subscript, df_stride);
                  buf->offset = glsl_dataflow_construct_binary_op(DATAFLOW_ADD, buf->offset, offset);
               } else {
                  /* Subscripting a swizzle. Construct an if-then-else tree */
                  /* TODO: There are special cases where we could generate better code.
                   * Since noone will ever write code like this, I haven't done it */
                  assert(buf->layout->flavour == LAYOUT_PRIM_NONMATRIX);
                  int stride = buf->layout->u.prim_nonmatrix_layout.stride;
                  Dataflow *swizzled_subscript = subscript_apply_swizzle(subscript, buf->swizzle);
                  buf->layout = glsl_mem_prim_nonmatrix_layout(0);
                  buf->offset = glsl_dataflow_construct_binary_op(DATAFLOW_MUL, swizzled_subscript, glsl_dataflow_construct_const_uint(stride));
                  buf->swizzle = NULL;     /* The old swizzle has been taken care of already */
               }
            }
         }
         break;

      default:
         unreachable();
         return;
   }
}

static void do_in_load(Dataflow *address, Dataflow *offset,
                       Dataflow **result, DataflowType t, unsigned count, unsigned stride)
{
   for (unsigned i=0; i<count; i++) {
      Dataflow *off = glsl_dataflow_construct_binary_op(DATAFLOW_SHR, offset, glsl_dataflow_construct_const_uint(2));
      off = glsl_dataflow_construct_binary_op(DATAFLOW_ADD, off, glsl_dataflow_construct_const_uint(i * stride));
      Dataflow *addr = glsl_dataflow_construct_binary_op(DATAFLOW_ADD, address, off);
      Dataflow *a = glsl_dataflow_construct_vec4(addr, NULL, NULL, NULL);
      result[i] = glsl_dataflow_construct_address_load(DATAFLOW_IN_LOAD, t, a);
   }
}

static void in_load(BasicBlock *b, const struct buf_ref *buf, Dataflow *offset,
                    Dataflow **result, MemLayout *layout, SymbolType *type)
{
   (void)b;

   assert(layout->flavour == LAYOUT_PRIM_MATRIX || layout->flavour == LAYOUT_PRIM_NONMATRIX);
   if (layout->flavour == LAYOUT_PRIM_MATRIX) {
      unsigned columns = glsl_prim_matrix_type_subscript_dimensions(type->u.primitive_type.index, 0);
      unsigned rows    = glsl_prim_matrix_type_subscript_dimensions(type->u.primitive_type.index, 1);

      unsigned matrix_stride = layout->u.prim_matrix_layout.stride;
      for (unsigned i=0; i<columns; i++) {
         Dataflow *vec_offset = glsl_dataflow_construct_binary_op(DATAFLOW_ADD, offset, glsl_dataflow_construct_const_uint(i*matrix_stride));
         do_in_load(buf->address, vec_offset, result+i*rows, df_scalar_type(type, 0), rows, 1);
      }
   } else {
      unsigned stride = layout->u.prim_nonmatrix_layout.stride/4u;
      do_in_load(buf->address, offset, result,
                 df_scalar_type(type, 0), type->scalar_count, stride);
   }
}

void glsl_buffer_load_calculate_dataflow(BasicBlock *ctx, Dataflow **result,
                                         Dataflow *address, Dataflow *offset,
                                         MemLayout *layout, SymbolType *type,
                                         StorageQualifier sq)
{
   struct buf_ref b = {
      .address = address,
      .offset  = offset,
      .layout  = layout,
      .swizzle = NULL,
      .sq      = sq
   };

   lower_loads_stores(ctx, &b, b.offset, result, b.layout, type, build_loads);
}

void glsl_buffer_store_calculate_dataflow(BasicBlock *ctx, Dataflow **data,
                                          Dataflow *address, Dataflow *offset,
                                          MemLayout *layout, SymbolType *type,
                                          StorageQualifier sq)
{
   struct buf_ref b = {
      .address = address,
      .offset  = offset,
      .layout  = layout,
      .swizzle = NULL,
      .sq      = sq
   };

   lower_loads_stores(ctx, &b, b.offset, data, b.layout, type, build_stores);
}

static void buffer_load_expr_calculate_dataflow(BasicBlock *ctx, Dataflow **result, Expr *expr)
{
   struct buf_ref b = { 0, };
   calculate_buffer_address(ctx, &b, expr);

   LoadStoreFunc load_func = (b.sq != STORAGE_IN && b.sq != STORAGE_OUT) ? build_loads : in_load;
   lower_loads_stores(ctx, &b, b.offset, result, b.layout, expr->type, load_func);
}

void buffer_store_expr_calculate_dataflow(BasicBlock *ctx, Expr *lvalue_expr, Dataflow **values)
{
   struct buf_ref b = { 0, };
   calculate_buffer_address(ctx, &b, lvalue_expr);
   lower_loads_stores(ctx, &b, b.offset, values, b.layout, lvalue_expr->type, build_stores);
}

/* Return whether this expr lives in a buffer (UBO or SSBO) */
static bool instance_in_buffer(Symbol *symbol)
{
   if (symbol->flavour == SYMBOL_TEMPORARY || symbol->flavour == SYMBOL_PARAM_INSTANCE)
      return false;

   assert(symbol->flavour==SYMBOL_VAR_INSTANCE);

   return symbol->u.var_instance.block_info_valid &&
          ((symbol->u.var_instance.storage_qual == STORAGE_IN) ||
           symbol->u.var_instance.block_info.block_symbol);
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

static bool instance_requires_constant_index(const Symbol *s) {
   if (s->flavour == SYMBOL_TEMPORARY || s->flavour == SYMBOL_PARAM_INSTANCE)
      return false;

   return (s->u.var_instance.storage_qual == STORAGE_IN) &&
          (g_ShaderFlavour == SHADER_VERTEX || g_ShaderFlavour == SHADER_FRAGMENT);
}

static bool requires_constant_index(const Expr *expr) {
   switch (expr->flavour) {
      case EXPR_INSTANCE:
         return instance_requires_constant_index(expr->u.instance.symbol);
      case EXPR_SUBSCRIPT:
         return requires_constant_index(expr->u.subscript.aggregate);
      case EXPR_FIELD_SELECTOR:
         return requires_constant_index(expr->u.field_selector.aggregate);
      case EXPR_SWIZZLE:
         return requires_constant_index(expr->u.swizzle.aggregate);
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
static inline void expr_calculate_dataflow_subscript(BasicBlock *ctx, Dataflow **scalar_values, Expr *expr)
{
   if (!requires_constant_index(expr) && !glsl_type_contains_opaque(expr->type) && in_addressable_memory(expr)) {
      buffer_load_expr_calculate_dataflow(ctx, scalar_values, expr);
   } else {
      Dataflow **aggregate_scalar_values = alloc_dataflow(expr->u.subscript.aggregate->type->scalar_count);
      glsl_expr_calculate_dataflow(ctx, aggregate_scalar_values, expr->u.subscript.aggregate);
      int member_count = expr->u.subscript.aggregate->type->scalar_count / expr->type->scalar_count;

      Dataflow *subscript_scalar;
      assert(expr->u.subscript.subscript->type->scalar_count == 1);
      glsl_expr_calculate_dataflow(ctx, &subscript_scalar, expr->u.subscript.subscript);
      assert(glsl_dataflow_is_integral_type(subscript_scalar));

      if (member_count > 1 && glsl_prim_is_prim_atomic_type(expr->type))
      {
         Dataflow *addr = aggregate_scalar_values[0];
         Dataflow *index = clamp_subscript_index(subscript_scalar, member_count);
         Dataflow *stride_log2 = glsl_dataflow_construct_const_value(subscript_scalar->type, 2);
         Dataflow *offset = glsl_dataflow_construct_binary_op(DATAFLOW_SHL, index, stride_log2);
         *scalar_values = glsl_dataflow_construct_binary_op(DATAFLOW_ADD, addr, offset);
      }
      else if (member_count > 1 && glsl_prim_is_prim_comb_sampler_type(expr->type))
      {
         if (subscript_scalar->flavour == DATAFLOW_CONST) {
            scalar_values[0] = aggregate_scalar_values[subscript_scalar->u.constant.value];
            scalar_values[1] = aggregate_scalar_values[subscript_scalar->u.constant.value + expr->u.subscript.aggregate->type->u.array_type.member_count];
         } else {
#if V3D_VER_AT_LEAST(4,1,34,0)
            Dataflow *a[2] = { aggregate_scalar_values[0], aggregate_scalar_values[member_count] };
            Dataflow *stride_log2 = glsl_dataflow_construct_const_value(DF_UINT, gfx_log2(64));
            Dataflow *index = clamp_subscript_index(subscript_scalar, member_count);
            Dataflow *offset = glsl_dataflow_construct_binary_op(DATAFLOW_SHL, index, stride_log2);

            Dataflow *unnorm_bits = glsl_dataflow_construct_sampler_unnorms(a[1]);
            Dataflow *unnorm_bit = glsl_dataflow_construct_binary_op(DATAFLOW_ROR, unnorm_bits, index);
            unnorm_bit = glsl_dataflow_construct_binary_op(DATAFLOW_BITWISE_AND, unnorm_bit, glsl_dataflow_construct_const_value(DF_UINT, 2));

            for (unsigned i = 0; i != 2; ++i) {
               Dataflow *addr = glsl_dataflow_construct_reinterp(a[i], DF_UINT);
               if (i == 1) // Correct unnorm bit in tmuparam1.
                  addr = glsl_dataflow_construct_binary_op(DATAFLOW_BITWISE_XOR, addr, unnorm_bit);
               addr = glsl_dataflow_construct_binary_op(DATAFLOW_ADD, addr, offset);
               scalar_values[i] = glsl_dataflow_construct_reinterp(addr, a[i]->type);
            }
#else
            assert(0);  /* Dynamic indexing not supported on pre v4.0 */
#endif
         }
      }
      else if (subscript_scalar->flavour == DATAFLOW_CONST && expr->u.subscript.aggregate->type->scalar_count > 0)
      {
         /* If the index is out of range, clamp to max */
         unsigned scalar_index = gfx_umin(subscript_scalar->u.constant.value, member_count - 1) * expr->type->scalar_count;
         memcpy(scalar_values, aggregate_scalar_values + scalar_index, expr->type->scalar_count * sizeof(Dataflow*));
      }
      else
      {
         assert(expr->u.subscript.aggregate->type->flavour == SYMBOL_ARRAY_TYPE ||
                expr->u.subscript.aggregate->type->flavour == SYMBOL_PRIMITIVE_TYPE);

         memcpy(scalar_values, aggregate_scalar_values, expr->type->scalar_count * sizeof(Dataflow*));
         for (int j=1; j<member_count; j++) {
            Dataflow *cond = glsl_dataflow_construct_binary_op(DATAFLOW_EQUAL, subscript_scalar,
                                                               glsl_dataflow_construct_const_value(subscript_scalar->type, j));
            for (unsigned i=0; i<expr->type->scalar_count; i++) {
               scalar_values[i] = glsl_dataflow_construct_ternary_op(DATAFLOW_CONDITIONAL, cond,
                                                                     aggregate_scalar_values[j*expr->type->scalar_count + i],
                                                                     scalar_values[i]);
            }
         }
      }

      free_dataflow(aggregate_scalar_values);
   }
}

static inline void expr_calculate_dataflow_sequence(BasicBlock *ctx, Dataflow **scalar_values, Expr *expr)
{
   Dataflow **all_these_scalar_values = alloc_dataflow(expr->u.sequence.all_these->type->scalar_count);
   glsl_expr_calculate_dataflow(ctx, all_these_scalar_values, expr->u.sequence.all_these);
   glsl_expr_calculate_dataflow(ctx, scalar_values, expr->u.sequence.then_this);
   free_dataflow(all_these_scalar_values);
}

static inline void expr_calculate_dataflow_array_length(BasicBlock *ctx, Dataflow **scalar_values, Expr *expr)
{
   struct buf_ref b = { 0, };
   calculate_buffer_address(ctx, &b, expr->u.array_length.array);
   assert(b.sq == STORAGE_BUFFER);
   assert(b.address->flavour == DATAFLOW_ADDRESS);
   b.address = b.address->d.unary_op.operand;

   // There can only be one dynamically sized array in this buffer.
   Dataflow *array_length = glsl_dataflow_construct_buf_array_length(b.address, 0);
   scalar_values[0] = glsl_dataflow_construct_reinterp(array_length, DF_INT);
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
         unreachable();
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

static void expr_calculate_dataflow_unary_op(BasicBlock *ctx, Dataflow **scalar_values, Expr *expr)
{
   Expr      *operand = expr->u.unary_op.operand;
   Dataflow **operand_scalar_values = alloc_dataflow(operand->type->scalar_count);

   // Recurse.
   glsl_expr_calculate_dataflow(ctx, operand_scalar_values, operand);

   // Work out dataflow flavour.
   DataflowFlavour dataflow_flavour;
   switch (expr->flavour)
   {
   case EXPR_LOGICAL_NOT:  dataflow_flavour = DATAFLOW_LOGICAL_NOT;  break;
   case EXPR_BITWISE_NOT:  dataflow_flavour = DATAFLOW_BITWISE_NOT;  break;
   case EXPR_ARITH_NEGATE: dataflow_flavour = DATAFLOW_ARITH_NEGATE; break;
   default: unreachable(); return;
   }

   // Create nodes.
   for (unsigned i = 0; i < expr->type->scalar_count; i++)
      scalar_values[i] = glsl_dataflow_construct_unary_op(dataflow_flavour, operand_scalar_values[i]);

   free_dataflow(operand_scalar_values);
}

static inline void construct_dataflow_matXxY_mul_matZxY_matXxZ(int X, int Y, int Z, Dataflow **result_scalar_values,
                                                               Dataflow **left_scalar_values, Dataflow **right_scalar_values)
{
   /* Linear algebraic matZxY * matXxZ. */
   for(int i = 0; i < X; i++) {
      for (int j = 0; j < Y; j++) {
         Dataflow *acc = NULL;

         for (int k = 0; k < Z; k++) {
            Dataflow *mul = glsl_dataflow_construct_binary_op(DATAFLOW_MUL,
                                                              left_scalar_values[k*Y + j],
                                                              right_scalar_values[i*Z + k]);

            if (k == 0) acc = mul;
            else        acc = glsl_dataflow_construct_binary_op(DATAFLOW_ADD, acc, mul);
         }

         result_scalar_values[j + i*Y] = acc;
      }
   }
}

static inline void expr_calculate_dataflow_binary_op_arithmetic(BasicBlock *ctx, Dataflow **scalar_values, Expr *expr)
{
   Expr *left  = expr->u.binary_op.left;
   Expr *right = expr->u.binary_op.right;
   ExprFlavour flavour = expr->flavour;
   Dataflow **left_scalars  = alloc_dataflow(left->type->scalar_count);
   Dataflow **right_scalars = alloc_dataflow(right->type->scalar_count);

   // Recurse.
   glsl_expr_calculate_dataflow(ctx, left_scalars, left);
   glsl_expr_calculate_dataflow(ctx, right_scalars, right);

   DataflowFlavour df_flavour;
   switch (flavour) {
   case EXPR_MUL: df_flavour = DATAFLOW_MUL; break;
   case EXPR_DIV: df_flavour = DATAFLOW_DIV; break;
   case EXPR_REM: df_flavour = DATAFLOW_REM; break;
   case EXPR_ADD: df_flavour = DATAFLOW_ADD; break;
   case EXPR_SUB: df_flavour = DATAFLOW_SUB; break;
   default: unreachable(); return;
   }

   PrimitiveTypeIndex left_index  = left->type->u.primitive_type.index;
   PrimitiveTypeIndex right_index = right->type->u.primitive_type.index;
   PRIMITIVE_TYPE_FLAGS_T left_flags  = primitiveTypeFlags[left_index];
   PRIMITIVE_TYPE_FLAGS_T right_flags = primitiveTypeFlags[right_index];

   // From spec, the two operands must be:
   // 1 - the same type (the type being integer scalar/vector, float scalar/vector/matrix),
   // 2 - or one can be a scalar float and the other a float vector or matrix,
   // 3 - or one can be a scalar integer and the other an integer vector,
   // 4 - or, for multiply, one can be a float vector and the other a float matrix with the same dimensional size.
   // All operations are component-wise except EXPR_MUL involving at least one matrix (case 4).
   // The AST has checked that all combinations are valid, so we assert that usage is correct.

   // Case 1. Matrix * Matrix gets dealt with in case 4.
   if (left->type == right->type && !(flavour == EXPR_MUL && (left_flags & PRIM_MATRIX_TYPE)))
   {
      // Component-wise on same scalar type, same scalar count.
      for (unsigned i = 0; i < expr->type->scalar_count; i++)
         scalar_values[i] = glsl_dataflow_construct_binary_op(df_flavour, left_scalars[i], right_scalars[i]);
   }
   // Cases 2 and 3. Component-wise on same scalar type, different scalar counts.
   else if ( (left_flags & PRIM_SCALAR_TYPE) && (right_flags & (PRIM_VECTOR_TYPE | PRIM_MATRIX_TYPE)))
   {
      for (unsigned i = 0; i < expr->type->scalar_count; i++)
         scalar_values[i] = glsl_dataflow_construct_binary_op(df_flavour, left_scalars[0], right_scalars[i]);
   }
   else if ( (right_flags & PRIM_SCALAR_TYPE) && (left_flags & (PRIM_VECTOR_TYPE | PRIM_MATRIX_TYPE)))
   {
      for (unsigned i = 0; i < expr->type->scalar_count; i++)
         scalar_values[i] = glsl_dataflow_construct_binary_op(df_flavour, left_scalars[i], right_scalars[0]);
   }
   // Case 4. Multiplication with matrices.
   else {
      assert(flavour == EXPR_MUL);
      int X, Y, Z;
      if ( (left_flags & PRIM_MATRIX_TYPE) && (right_flags & PRIM_VECTOR_TYPE) ) {
         /* Linear algebraic mat * vec. */
         assert(glsl_prim_matrix_type_subscript_dimensions(left_index, 0) == primitiveTypeSubscriptDimensions[right_index]);
         X = 1;
         Y = primitiveTypeSubscriptDimensions[expr->type->u.primitive_type.index];
         Z = glsl_prim_matrix_type_subscript_dimensions(left_index, 0);
      } else if ( (right_flags & PRIM_MATRIX_TYPE) && (left_flags & PRIM_VECTOR_TYPE) ) {
         /* Linear algebraic vec * mat. */
         assert(primitiveTypeSubscriptDimensions[left_index] == glsl_prim_matrix_type_subscript_dimensions(right_index, 1));
         X = primitiveTypeSubscriptDimensions[expr->type->u.primitive_type.index];
         Y = 1;
         Z = glsl_prim_matrix_type_subscript_dimensions(right_index, 1);
      } else if ( (right_flags & PRIM_MATRIX_TYPE) && (left_flags & PRIM_MATRIX_TYPE) ) {
         /* Linear algebraic mat * mat. */
         assert(glsl_prim_matrix_type_subscript_dimensions(left_index, 0) == glsl_prim_matrix_type_subscript_dimensions(right_index, 1));
         X = glsl_prim_matrix_type_subscript_dimensions(expr->type->u.primitive_type.index, 0);
         Y = glsl_prim_matrix_type_subscript_dimensions(expr->type->u.primitive_type.index, 1);
         Z = glsl_prim_matrix_type_subscript_dimensions(right_index, 1);
      }
      else
         unreachable();

      construct_dataflow_matXxY_mul_matZxY_matXxZ(X, Y, Z, scalar_values, left_scalars, right_scalars);
   }

   free_dataflow(right_scalars);
   free_dataflow(left_scalars);
   return;
}

static void expr_calculate_dataflow_binary_op_common(BasicBlock *ctx, Dataflow **scalar_values, Expr *expr)
{
   Expr *left  = expr->u.binary_op.left;
   Expr *right = expr->u.binary_op.right;

   Dataflow **left_scalar_values  = alloc_dataflow(left->type->scalar_count);
   Dataflow **right_scalar_values = alloc_dataflow(right->type->scalar_count);

   glsl_expr_calculate_dataflow(ctx, left_scalar_values, left);
   glsl_expr_calculate_dataflow(ctx, right_scalar_values, right);

   DataflowFlavour flavour;
   switch (expr->flavour) {
      case EXPR_SHL:                flavour = DATAFLOW_SHL;                break;
      case EXPR_SHR:                flavour = DATAFLOW_SHR;                break;
      case EXPR_LOGICAL_XOR:        flavour = DATAFLOW_LOGICAL_XOR;        break;
      case EXPR_LESS_THAN:          flavour = DATAFLOW_LESS_THAN;          break;
      case EXPR_LESS_THAN_EQUAL:    flavour = DATAFLOW_LESS_THAN_EQUAL;    break;
      case EXPR_GREATER_THAN:       flavour = DATAFLOW_GREATER_THAN;       break;
      case EXPR_GREATER_THAN_EQUAL: flavour = DATAFLOW_GREATER_THAN_EQUAL; break;
      case EXPR_BITWISE_AND:        flavour = DATAFLOW_BITWISE_AND;        break;
      case EXPR_BITWISE_OR:         flavour = DATAFLOW_BITWISE_OR;         break;
      case EXPR_BITWISE_XOR:        flavour = DATAFLOW_BITWISE_XOR;        break;
      default: unreachable(); break;
   }

   for (unsigned i=0; i<expr->type->scalar_count; i++) {
      int left_idx  = (left->type->scalar_count  == 1) ? 0 : i;
      int right_idx = (right->type->scalar_count == 1) ? 0 : i;
      scalar_values[i] = glsl_dataflow_construct_binary_op(flavour,
                                                           left_scalar_values[left_idx],
                                                           right_scalar_values[right_idx]);
   }

   free_dataflow(right_scalar_values);
   free_dataflow(left_scalar_values);
}

static inline void expr_calculate_dataflow_binary_op_equality(BasicBlock *ctx, Dataflow **scalar_values, Expr *expr)
{
   Expr *left = expr->u.binary_op.left;
   Expr *right = expr->u.binary_op.right;
   Dataflow **left_scalar_values  = alloc_dataflow(left->type->scalar_count);
   Dataflow **right_scalar_values = alloc_dataflow(right->type->scalar_count);
   Dataflow *acc = NULL;

   // Recurse.
   glsl_expr_calculate_dataflow(ctx, left_scalar_values,  left);
   glsl_expr_calculate_dataflow(ctx, right_scalar_values, right);

   assert(left->type->scalar_count == right->type->scalar_count);

   for (unsigned i = 0; i < left->type->scalar_count; i++) {
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
         unreachable();
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
static inline void expr_calculate_dataflow_function_call(BasicBlock *ctx, Dataflow **scalar_values, Expr *expr)
{
   Symbol *function = expr->u.function_call.function;
   const NStmt *function_def = function->u.function_instance.function_norm_def;
   const NStmt *return_stmt = function_def->u.function_def.body->head->v;
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

      case EXPR_SHL:
      case EXPR_SHR:
      case EXPR_LOGICAL_XOR:
      case EXPR_LESS_THAN:
      case EXPR_LESS_THAN_EQUAL:
      case EXPR_GREATER_THAN:
      case EXPR_GREATER_THAN_EQUAL:
      case EXPR_BITWISE_AND:
      case EXPR_BITWISE_OR:
      case EXPR_BITWISE_XOR:
         expr_calculate_dataflow_binary_op_common(ctx, scalar_values, expr);
         break;

      case EXPR_EQUAL:
      case EXPR_NOT_EQUAL:
         expr_calculate_dataflow_binary_op_equality(ctx, scalar_values, expr);
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
         unreachable();
         break;

      // There should not be any of these for which expr->compile_time_value is NULL.
      case EXPR_VALUE:
      default:
         unreachable();
         break;
      }
   }
}
