/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "interface/khronos/common/khrn_int_common.h"
#include "interface/khronos/common/khrn_int_parallel.h"

#include "interface/khronos/common/khrn_client_platform.h"
#include "interface/khronos/common/khrn_client_vector.h"

#include <stdlib.h>
#include <string.h>

void khrn_vector_init(KHRN_VECTOR_T *vector, uint32_t capacity)
{
   vector->data = (capacity == 0) ? NULL : malloc(capacity);
   vector->capacity = vector->data ? capacity : 0;
   vector->size = 0;
}

void khrn_vector_term(KHRN_VECTOR_T *vector)
{
   free(vector->data);
}

bool khrn_vector_extend(KHRN_VECTOR_T *vector, uint32_t size)
{
   uint32_t req_capacity = vector->size + size;
   if (req_capacity > vector->capacity) {
      uint32_t new_capacity = _max(vector->capacity + (vector->capacity >> 1), req_capacity);
      void *new_data = malloc(new_capacity);
      if (!new_data) {
         new_capacity = req_capacity;
         new_data = malloc(new_capacity);
         if (!new_data) {
            return false;
         }
      }
      if (vector->data) {
         khrn_par_memcpy(new_data, vector->data, vector->size);
         free(vector->data);
      }
      vector->data = new_data;
      vector->capacity = new_capacity;
   }
   vector->size += size;
   return true;
}

void khrn_vector_clear(KHRN_VECTOR_T *vector)
{
   free(vector->data);
   vector->data = NULL;
   vector->capacity = 0;
   vector->size = 0;
}
