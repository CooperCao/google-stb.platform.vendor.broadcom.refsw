/*=============================================================================
Copyright (c) 2013 Broadcom Europe Limited.
All rights reserved.

Project  :  khronos

FILE DESCRIPTION
This is a basic implementation of a std::vector for storing uintptr
=============================================================================*/
#include "middleware/khronos/common/khrn_uintptr_vector.h"

static bool data_alloc(KHRN_UINTPTR_VECTOR_T *vector, int grow_size)
{
   uintptr_t *new = realloc(vector->data, (vector->capacity + grow_size) * sizeof(uintptr_t));
   if (new == NULL)
      return false;
   vector->data = new;
   vector->capacity += grow_size;
   return true;
}

bool khrn_uintptr_vector_init(KHRN_UINTPTR_VECTOR_T *vector, unsigned int
      initial_capacity, unsigned int chunk_size)
{
   /* don't allow chunk_size 0 */
   if (chunk_size == 0)
      return false;
   memset(vector, 0, sizeof(KHRN_UINTPTR_VECTOR_T));
   vector->chunk_size = chunk_size;
   return data_alloc(vector, initial_capacity);
}

void khrn_uintptr_vector_destroy(KHRN_UINTPTR_VECTOR_T *vector)
{
   if (!vector)
      return;
   free(vector->data);
   memset(vector, 0, sizeof(KHRN_UINTPTR_VECTOR_T));
}

bool khrn_uintptr_vector_push_back(KHRN_UINTPTR_VECTOR_T *vector, uintptr_t elem)
{
   if (vector->size == vector->capacity)
   {
      if (!data_alloc(vector, vector->chunk_size))
         return false;
   }
   vector->data[vector->size] = elem;
   vector->size++;
   return true;
}
