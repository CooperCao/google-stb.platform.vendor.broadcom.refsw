/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "khrn_uintptr_vector.h"

bool khrn_uintptr_vector_push_back(khrn_uintptr_vector *vector, uintptr_t elem)
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