/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include <stdlib.h>
#include <string.h>

#include "glsl_safemem.h"
#include "glsl_dataflow_visitor.h"

void glsl_dataflow_visitor_begin(DataflowVisitor *visitor)
{
   visitor->seen_array_size = 0;
   visitor->seen = NULL;
}

void glsl_dataflow_visitor_end(DataflowVisitor *visitor)
{
   glsl_safemem_free(visitor->seen);

   visitor->seen_array_size = 0;
   visitor->seen = NULL;
}

static void resize(DataflowVisitor *visitor, int min_size)
{
   // Increase the array capacity by at least 50%
   int old_array_size = visitor->seen_array_size;
   visitor->seen_array_size = min_size + min_size / 2;
   assert(visitor->seen_array_size > old_array_size);

   visitor->seen = glsl_safemem_realloc(visitor->seen, sizeof(Dataflow *) * visitor->seen_array_size);
   memset(visitor->seen + old_array_size, 0, sizeof(Dataflow *) * (visitor->seen_array_size - old_array_size));
}

void glsl_dataflow_visitor_accept(DataflowVisitor *visitor, Dataflow *dataflow, void *data, DataflowPreVisitor dprev, DataflowPostVisitor dpostv)
{
   if (dataflow == NULL)
      return;

   if (dataflow->id >= visitor->seen_array_size)
      resize(visitor, dataflow->id + 1);

   if (visitor->seen[dataflow->id] == NULL) {
      visitor->seen[dataflow->id] = dataflow;

      if (dprev) {
         Dataflow *alt_dataflow = dprev(dataflow, data);

         if (alt_dataflow != dataflow) {
            glsl_dataflow_visitor_accept(visitor, alt_dataflow, data, dprev, dpostv);
            return;
         }
      }

      for (int i=0; i<dataflow->dependencies_count; ++i) {
         glsl_dataflow_visitor_accept(visitor, dataflow->d.dependencies[i], data, dprev, dpostv);
      }

      if (dpostv) dpostv(dataflow, data);
   } else {
      // Check that we have not seen two dataflows with the same id
      assert(visitor->seen[dataflow->id] == dataflow);
   }
}

void glsl_dataflow_visit_array(Dataflow **dataflow, int start, int end, void *data, DataflowPreVisitor dprev, DataflowPostVisitor dpostv)
{
   DataflowVisitor pass;

   glsl_dataflow_visitor_begin(&pass);
   for (int i = start; i < end; i++) {
      glsl_dataflow_visitor_accept(&pass, dataflow[i], data, dprev, dpostv);
   }
   glsl_dataflow_visitor_end(&pass);
}

void glsl_dataflow_reloc_visitor_begin(DataflowRelocVisitor *visitor,
                                       Dataflow *df_arr, int df_count,
                                       bool *temp)
{
   visitor->df_arr   = df_arr;
   visitor->df_count = df_count;
   visitor->seen = temp;
   memset(visitor->seen, 0, df_count * sizeof(bool));
}

void glsl_dataflow_reloc_visitor_end(DataflowRelocVisitor *visitor) {
   (void)(visitor);
}

void glsl_dataflow_reloc_post_visitor(DataflowRelocVisitor *visitor, int id, void *data, DataflowPostVisitor dpostv)
{
   if (id == -1) return;
   assert(id < visitor->df_count);

   if (visitor->seen[id]) return;
   visitor->seen[id] = true;

   Dataflow *dataflow = &visitor->df_arr[id];

   for (int i=0; i<dataflow->dependencies_count; ++i) {
      glsl_dataflow_reloc_post_visitor(visitor, dataflow->d.reloc_deps[i], data, dpostv);
   }

   dpostv(dataflow, data);
}

void glsl_dataflow_reloc_visit_array(Dataflow *dataflow, int dataflow_count, int *ids, int start, int end, void *data, DataflowPostVisitor dpostv, bool *temp)
{
   DataflowRelocVisitor pass;

   glsl_dataflow_reloc_visitor_begin(&pass, dataflow, dataflow_count, temp);
   for (int i = start; i < end; i++) {
      glsl_dataflow_reloc_post_visitor(&pass, ids[i], data, dpostv);
   }
   glsl_dataflow_reloc_visitor_end(&pass);
}
