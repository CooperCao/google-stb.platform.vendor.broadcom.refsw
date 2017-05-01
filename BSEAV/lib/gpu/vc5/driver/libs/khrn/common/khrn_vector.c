/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "khrn_vector.h"

bool khrn_vector_grow(khrn_vector* vector, size_t item_size)
{
   assert(vector->size == vector->capacity);
   size_t new_capacity = vector->capacity ? ((vector->capacity * 3) / 2) : 64;
   void *new_data = realloc(vector->data, new_capacity * item_size);
   if (!new_data)
      return false;
   vector->data = new_data;
   vector->capacity = new_capacity;
   return true;
}
