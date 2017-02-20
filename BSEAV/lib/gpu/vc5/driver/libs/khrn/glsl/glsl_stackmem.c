/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "glsl_stackmem.h"
#include "glsl_errors.h"
#include "libs/util/common.h"
#include <stdint.h>
#include <stdlib.h>
#include <stddef.h>
#include <assert.h>

typedef struct glsl_allocation_list_s {
   struct glsl_allocation_list_s *next;

   /*
      The alignment of this array is of particular concern, but there
      is no way to guarantee it short of a second malloc, which is not
      desired. Most likely, a pointer has as restrictive alignment
      requirements as anything else (barring SIMD instructions) and so
      the address following a pointer is also well aligned. This might
      not always hold, and could cause unpredictable behaviour in
      future. However, there are no alternatives that I can see
      without another malloc.
   */
   uint8_t data[1];
} glsl_allocation_list_t;

static glsl_allocation_list_t *allocations = NULL;

void *glsl_stack_malloc(size_t element_size, size_t element_count) {
   size_t required_mem = element_size * element_count;
   glsl_allocation_list_t *new_alloc;
   new_alloc = malloc(offsetof(glsl_allocation_list_t,data) + required_mem);
   if(!new_alloc) {
      free(new_alloc);
      glsl_compile_error(ERROR_CUSTOM, 6, 0, NULL);
      return NULL;
   }
   new_alloc->next = allocations;
   allocations     = new_alloc;
   return &new_alloc->data;
}

void glsl_stack_free(void *v) {
   assert(v == &allocations->data);

   glsl_allocation_list_t *cur = allocations;
   allocations = allocations->next;
   free(cur);
}

int glsl_stack_cleanup() {
   glsl_allocation_list_t *cur;
   glsl_allocation_list_t *next;

   int frees = 0;
   for(cur = allocations; cur; cur = next) {
      next = cur->next;

      ++frees;
      free(cur);
   }
   allocations = NULL;
   return frees;
}
