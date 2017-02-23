/*=============================================================================
Broadcom Proprietary and Confidential. (c)2014 Broadcom.
All rights reserved.

Project  :  glsl
Module   :

FILE DESCRIPTION
=============================================================================*/

#include <stdlib.h>

#include "glsl_map.h"
#include "glsl_backflow.h"
#include "glsl_backflow_visitor.h"

struct backflow_visitor {
   BackflowPreVisitor  prev;
   BackflowPostVisitor postv;
   void *data;
   Map *map;
};

static bool already_visited(const struct backflow_visitor *v, const Backflow *b) {
   return glsl_map_get(v->map, b) != NULL;
}

static void mark_visited(struct backflow_visitor *v, Backflow *b) {
   glsl_map_put(v->map, b, b);
}

void glsl_backflow_visit(Backflow *backflow, struct backflow_visitor *v)
{
   if (!backflow || already_visited(v, backflow)) return;

   mark_visited(v, backflow);

   if (v->prev) {
      Backflow *alt = v->prev(backflow, v->data);

      if (alt != backflow) {
         glsl_backflow_visit(alt, v);
         return;
      }
   }

   for (int i=0; i<BACKFLOW_DEP_COUNT; ++i)
      glsl_backflow_visit(backflow->dependencies[i], v);

   BackflowChainNode *node;
   LIST_FOR_EACH(node, &backflow->io_dependencies, l)
      glsl_backflow_visit(node->ptr, v);

   if (v->postv) v->postv(backflow, v->data);
}

struct backflow_visitor *glsl_backflow_visitor_begin(void *data, BackflowPreVisitor prev, BackflowPostVisitor postv) {
   struct backflow_visitor *v = glsl_safemem_malloc(sizeof(struct backflow_visitor));
   v->data  = data;
   v->prev  = prev;
   v->postv = postv;
   v->map   = glsl_map_new();
   return v;
}

void glsl_backflow_visitor_end(struct backflow_visitor *v) {
   glsl_safemem_free(v);
}
