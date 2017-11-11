/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include <string.h>

#include "glsl_globals.h"
#include "glsl_dataflow_cse.h"
#include "glsl_map.h"
#include "libs/util/gfx_util/gfx_util.h"

static_assrt(sizeof(const_value) == sizeof(uint32_t));

static inline bool is_buf_size_variant(const Dataflow *d) {
   return d->flavour == DATAFLOW_BUF_SIZE
       || d->flavour == DATAFLOW_BUF_ARRAY_LENGTH;
}

static uint32_t dataflow_hash(const Dataflow *df)
{
   const uint32_t p = 2654435761u;
   uint32_t hash = p * df->flavour;
   hash = p * (hash ^ df->type);
   // include non-const dependency pointers and constant values in the hash
   for (int i = 0; i < df->dependencies_count; i++) {
      const Dataflow *d = df->d.dependencies[i];
      if (d == NULL) continue;

      if (d->flavour == DATAFLOW_CONST)
         hash = p * (hash ^ d->u.constant.value);
      else if (d->flavour == DATAFLOW_UNIFORM)
         hash = p * (hash ^ (d->u.buffer.index | (d->u.buffer.offset << 16)));
      else if (is_buf_size_variant(d)) {
         hash = p * (hash ^ d->u.constant.value);
         hash = p * (hash ^ d->d.unary_op.operand->u.buffer.index);
      } else if (d->flavour == DATAFLOW_CONST_IMAGE)
         hash = p * (hash ^ ((uint32_t)d->u.const_image.is_32bit | (d->u.const_image.location << 1)));
      else if (d->flavour == DATAFLOW_CONST_SAMPLER)
         hash = p * (hash ^ d->u.linkable_value.row);
#if V3D_VER_AT_LEAST(4,0,2,0)
      else if (d->flavour == DATAFLOW_SAMPLER_UNNORMS)
         hash = p * (hash ^ d->d.unary_op.operand->u.linkable_value.row);
#endif
      else
         hash = p * (hash ^ (uint32_t)(uintptr_t)d);
   }

   for (int i=0; i<sizeof(df->u)/4; i++) {
      hash = p * (hash ^ df->u.raw[i]);
   }
   return hash;
}

static bool dataflow_equals(const Dataflow *df0, const Dataflow *df1)
{
   if (df0->flavour != df1->flavour) return false;
   if (df0->type != df1->type) return false;
   if (df0->dependencies_count != df1->dependencies_count) return false;
   for (int i = 0; i < df0->dependencies_count; i++) {
      Dataflow *dep0 = df0->d.dependencies[i];
      Dataflow *dep1 = df1->d.dependencies[i];
      if (dep0 && dep1 && dep0->flavour == dep1->flavour) {
         if (dep0->flavour == DATAFLOW_CONST &&
             dep0->type == dep1->type && dep0->u.constant.value == dep1->u.constant.value && df0->flavour != DATAFLOW_VEC4)
            continue; // consider equal if dep0 and dep1 are the same constant
         if (dep0->flavour == DATAFLOW_UNIFORM &&
             dep0->u.buffer.index  == dep1->u.buffer.index &&
             dep0->u.buffer.offset == dep1->u.buffer.offset)
            continue; // consider equal if dep0 and dep1 are the same uniform
         if (is_buf_size_variant(dep0) &&
            dep0->u.constant.value == dep1->u.constant.value &&
            dep0->d.unary_op.operand->u.buffer.index == dep1->d.unary_op.operand->u.buffer.index)
            continue; // consider equal if dep0 and dep1 are the same buffer size
         if (dep0->flavour == DATAFLOW_CONST_IMAGE &&
            dep0->u.const_image.location == dep1->u.const_image.location &&
            dep0->u.const_image.is_32bit == dep1->u.const_image.is_32bit)
            continue; // consider equal if dep0 and dep1 are the same image
         if (dep0->flavour == DATAFLOW_CONST_SAMPLER &&
            dep0->u.linkable_value.row == dep1->u.linkable_value.row)
            continue; // consider equal if dep0 and dep1 are the same sampler unnorm array
#if V3D_VER_AT_LEAST(4,0,2,0)
         if (dep0->flavour == DATAFLOW_SAMPLER_UNNORMS &&
            dep0->d.unary_op.operand->u.linkable_value.row == dep1->d.unary_op.operand->u.linkable_value.row)
            continue;
#endif
      }
      if (dep0 != dep1) return false;
   }
   if (memcmp(&df0->u, &df1->u, sizeof(df0->u)) != 0) return false;
   return true;
}

static bool df_type_is_read_only(DataflowFlavour f) {
   return f == DATAFLOW_UNIFORM || f == DATAFLOW_UNIFORM_BUFFER;
}

static bool addr_is_read_only(const Dataflow *p) {
   if (p->flavour == DATAFLOW_ADDRESS && df_type_is_read_only(p->d.unary_op.operand->flavour))
      return true;

   if (p->flavour == DATAFLOW_ADD) {
      const Dataflow *l = p->d.binary_op.left;
      const Dataflow *r = p->d.binary_op.right;
      if (l->flavour == DATAFLOW_ADDRESS && df_type_is_read_only(l->d.unary_op.operand->flavour))
         return true;
      if (r->flavour == DATAFLOW_ADDRESS && df_type_is_read_only(r->d.unary_op.operand->flavour))
         return true;
   }

   return false;
}

static Dataflow *dataflow_cse_accept(Map *ctx, Dataflow *df, bool assume_read_only)
{
   if (df == NULL) return NULL;

   // Constants are only partially supported to avoid confusing the backend.
   // Identical constants are not merged into a single dataflow node,
   // however the dataflow referencing them will still be merged if possible.

   // Exclude the following dataflow flavours
   switch (df->flavour) {
      case DATAFLOW_CONST:
      case DATAFLOW_CONST_IMAGE:
      case DATAFLOW_CONST_SAMPLER:
#if V3D_VER_AT_LEAST(4,0,2,0)
      case DATAFLOW_SAMPLER_UNNORMS:
#endif
      case DATAFLOW_UNIFORM:
      case DATAFLOW_STORAGE_BUFFER:
      case DATAFLOW_BUF_SIZE:
      case DATAFLOW_BUF_ARRAY_LENGTH:
#if !V3D_VER_AT_LEAST(4,0,2,0)
      case DATAFLOW_IMAGE_INFO_PARAM:
#endif
         return df;
      default:
         break;
   }

   // Early out if we can find this node without checking dependents. Needed for
   // performance, otherwise we visit nodes repeatedly.
   // It would be better to rewrite this to only visit nodes once.
   uint32_t hash = dataflow_hash(df);
   if (hash == 0) return df;

   Dataflow *duplicate;
   if ((duplicate = glsl_map_get(ctx, (void *)(uintptr_t)hash)))
      if (dataflow_equals(duplicate, df)) {
         duplicate->age = gfx_umin(duplicate->age, df->age);
         return duplicate;
      }

   // Depth first traversal of the graph
   for (int i = 0; i < df->dependencies_count; i++)
      df->d.dependencies[i] = dataflow_cse_accept(ctx, df->d.dependencies[i], assume_read_only);

   /* Old versions of the language didn't allow writes, in which case assume everything
    * is read-only. Otherwise we need to check */
   if (df->flavour == DATAFLOW_VECTOR_LOAD)
      if (!(assume_read_only || addr_is_read_only(df->d.unary_op.operand))) return df;

   // If we have seen identical node before, use it instead.
   hash = dataflow_hash(df);
   if (hash == 0) return df;
   if ((duplicate = glsl_map_get(ctx, (void *)(uintptr_t)hash))) {
      if (dataflow_equals(duplicate, df)) {
         duplicate->age = gfx_umin(duplicate->age, df->age);
         return duplicate;
      }
   }

   glsl_map_put(ctx, (void *)(uintptr_t)hash, df);
   return df;
}

static void block_cse(Dataflow **outputs, int n_outputs, bool assume_read_only)
{
   Map *ctx = glsl_map_new();
   for (int i=0; i<n_outputs; i++)
      outputs[i] = dataflow_cse_accept(ctx, outputs[i], assume_read_only);
   glsl_map_delete(ctx);
}

void glsl_dataflow_cse(SSABlock *block, int n_blocks, bool assume_read_only) {
   for (int i=0; i<n_blocks; i++)
      block_cse(block[i].outputs, block[i].n_outputs, assume_read_only);
}
