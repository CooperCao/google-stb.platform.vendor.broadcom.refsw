/*=============================================================================
Broadcom Proprietary and Confidential. (c)2013 Broadcom.
All rights reserved.

Project  :  khronos

FILE DESCRIPTION
This is a basic implementation of a std::vector for storing uintptr
=============================================================================*/
#include "khrn_uintptr_vector.h"

void khrn_uintptr_vector_init(KHRN_UINTPTR_VECTOR_T *vector)
{
   memset(vector, 0, sizeof(KHRN_UINTPTR_VECTOR_T));
}

void khrn_uintptr_vector_destroy(KHRN_UINTPTR_VECTOR_T *vector)
{
   free(vector->data);
   memset(vector, 0, sizeof(KHRN_UINTPTR_VECTOR_T));
}

bool khrn_uintptr_vector_push_back(KHRN_UINTPTR_VECTOR_T *vector, uintptr_t elem)
{
   if (vector->size == vector->capacity)
   {
      size_t new_capacity = vector->capacity ? ((vector->capacity * 3) / 2) : 64;
      uintptr_t *new_data = realloc(vector->data, new_capacity * sizeof(uintptr_t));
      if (!new_data)
         return false;
      vector->data = new_data;
      vector->capacity = new_capacity;
   }
   assert(vector->size < vector->capacity);
   vector->data[vector->size] = elem;
   vector->size++;
   return true;
}
