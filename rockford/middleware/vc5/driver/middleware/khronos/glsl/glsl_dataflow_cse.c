/*=============================================================================
Copyright (c) 2014 Broadcom Europe Limited.
All rights reserved.

Project  :  glsl
Module   :

FILE DESCRIPTION
=============================================================================*/

#include "glsl_dataflow_cse.h"
#include "glsl_map.h"

static unsigned int dataflow_hash(const Dataflow *df)
{
   const unsigned int p = 2654435761u;
   unsigned int hash = 0;
   hash = p * (hash ^ df->flavour);
   hash = p * (hash ^ df->type);
   // include non-const dependency pointers and constant values in the hash
   for (int i = 0; i < df->dependencies_count; i++) {
      Dataflow *d = df->d.dependencies[i];
      if (d && d->flavour != DATAFLOW_CONST)
         hash = p * (hash ^ (unsigned int)(uintptr_t)d);
      else if (d && d->flavour == DATAFLOW_CONST)
         hash = p * (hash ^ (unsigned int)d->u.constant.value);
   }
   if (df->flavour == DATAFLOW_GET_VEC4_COMPONENT) hash = p * (hash ^ df->u.get_vec4_component.component_index);
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
      if (dep0 && dep1 && dep0->flavour == DATAFLOW_CONST && dep1->flavour == DATAFLOW_CONST &&
          dep0->type == dep1->type && dep0->u.constant.value == dep1->u.constant.value)
         continue; // consider equal if dep0 and dep1 are the same constant
      if (dep0 != dep1) return false;
   }
   if (memcmp(&df0->u, &df1->u, sizeof(df0->u)) != 0) return false;
   return true;
}

static Dataflow *dataflow_cse_accept(Map *ctx, Dataflow *df)
{
   Dataflow *duplicate;
   unsigned int hash;

   if (df == NULL) return NULL;

   // Constants are only partially supported to avoid confusing the backend.
   // Identical constants are not merged into a single dataflow node,
   // however the dataflow referencing them will still be merged if possible.

   // Exclude the following dataflow flavours
   switch (df->flavour) {
      case DATAFLOW_CONST:
      case DATAFLOW_CONST_SAMPLER:
      case DATAFLOW_UNIFORM:
      case DATAFLOW_UNIFORM_BUFFER:
      case DATAFLOW_STORAGE_BUFFER:
      case DATAFLOW_IN:
         return df;
      default:
         break;
   }

   // Early out if we can find this node without checking dependents. Needed for
   // performance, otherwise we visit nodes repeatedly.
   // It would be better to rewrite this to only visit nodes once.
   hash = dataflow_hash(df);
   if (hash == 0) return df;
   if ((duplicate = glsl_map_get(ctx, (void *)(uintptr_t)hash)))
      if (dataflow_equals(duplicate, df))
         return duplicate;

   // Depth first traversal of the graph
   for (int i = 0; i < df->dependencies_count; i++)
      df->d.dependencies[i] = dataflow_cse_accept(ctx, df->d.dependencies[i]);

   // If we have seen identical node before, use it instead.
   hash = dataflow_hash(df);
   if (hash == 0) return df;
   if ((duplicate = glsl_map_get(ctx, (void *)(uintptr_t)hash))) {
      if (dataflow_equals(duplicate, df))
         return duplicate;
   }

   glsl_map_put(ctx, (void *)(uintptr_t)hash, df);
   return df;
}

static void block_cse(Dataflow **outputs, int n_outputs)
{
   Map *ctx = glsl_map_new();
   for (int i=0; i<n_outputs; i++)
      outputs[i] = dataflow_cse_accept(ctx, outputs[i]);
}

void glsl_dataflow_cse(SSABlock *block, int n_blocks) {
   for (int i=0; i<n_blocks; i++)
      block_cse(block[i].outputs, block[i].n_outputs);
}
