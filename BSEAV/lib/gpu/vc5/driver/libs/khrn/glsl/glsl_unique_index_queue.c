/*=============================================================================
Broadcom Proprietary and Confidential. (c)2014 Broadcom.
All rights reserved.

Project  :  glsl
Module   :

FILE DESCRIPTION
=============================================================================*/

#include "glsl_fastmem.h"

#include "glsl_unique_index_queue.h"

#include <string.h>
#include <assert.h>

struct glsl_unique_index_queue_s {
   int *queue;
   int  size;
   int  head;
   int  tail;
   int *mark;
};

GLSL_UNIQUE_INDEX_QUEUE_T *glsl_unique_index_queue_alloc(int size) {
   GLSL_UNIQUE_INDEX_QUEUE_T *ret;

   ret        = malloc_fast(sizeof(*ret));
   ret->queue = malloc_fast(sizeof(*ret->queue) * size);
   ret->mark  = malloc_fast(sizeof(*ret->mark)  * size);
   ret->size  = size;

   glsl_unique_index_queue_reset(ret);

   return ret;
}

void glsl_unique_index_queue_add(GLSL_UNIQUE_INDEX_QUEUE_T *uiq, int idx) {
   /* This is a fixed size queue for indices in the given size range */
   assert(idx >= 0 && idx < uiq->size);
   if(uiq->mark[idx] == 0) {
      assert(uiq->tail < uiq->size);

      uiq->queue[uiq->tail++] = idx;
   }
   uiq->mark[idx]++;
}

int glsl_unique_index_queue_remove(GLSL_UNIQUE_INDEX_QUEUE_T *uiq) {
   if(glsl_unique_index_queue_empty(uiq)) {
      return -1;
   }
   assert(uiq->head < uiq->size);
   return uiq->queue[uiq->head++];
}

void glsl_unique_index_queue_reset(GLSL_UNIQUE_INDEX_QUEUE_T *uiq) {
   uiq->head = 0;
   uiq->tail = 0;
   memset(uiq->mark, 0, sizeof(*uiq->mark) * uiq->size);
}

bool glsl_unique_index_queue_empty(GLSL_UNIQUE_INDEX_QUEUE_T *uiq) {
   return uiq->head == uiq->tail;
}
