/*=============================================================================
Broadcom Proprietary and Confidential. (c)2013 Broadcom.
All rights reserved.

Project  :  khronos

FILE DESCRIPTION
This is a basic implementation of a std::vector for storing uintptr
=============================================================================*/
#ifndef KHRN_UINTPTR_VECTOR_H
#define KHRN_UINTPTR_VECTOR_H

#include "vcos.h"
#include <stdbool.h>

typedef struct
{
   uintptr_t *data;
   unsigned int capacity;   /* number of allocated elements */
   unsigned int size;       /* number of used elements */
   unsigned int chunk_size; /* number of elements to grow with when we need
                               more space */
}
KHRN_UINTPTR_VECTOR_T;

/* initial_capacity will alocated data for "initial_capacity" elements; can be 0;
 * chunck_size specifies the number of elements to grow with when we need more
 * space; cannot be 0 */
extern bool khrn_uintptr_vector_init(KHRN_UINTPTR_VECTOR_T *vector, unsigned int
      initial_capacity, unsigned int chunk_size);

extern void khrn_uintptr_vector_destroy(KHRN_UINTPTR_VECTOR_T *vector);

/* inserts a new element at the end */
extern bool khrn_uintptr_vector_push_back(KHRN_UINTPTR_VECTOR_T *vector,
      uintptr_t elem);

/* return the number of used entries un the vector */
static inline unsigned int khrn_uintptr_vector_get_size(
      const KHRN_UINTPTR_VECTOR_T *vector)
{
   return vector->size;
}

/* returns element at position i; it is the users reasponsability to check that i is < vector's size */
static inline uintptr_t khrn_uintptr_vector_item(
      const KHRN_UINTPTR_VECTOR_T *vector, unsigned int i)
{
   assert(i < vector->size);
   return vector->data[i];
}

/* erases all the elements */
static inline void khrn_uintptr_vector_clear(KHRN_UINTPTR_VECTOR_T *vector)
{
   vector->size = 0;
}

/*  returns true if vector size is zero */
static inline bool khrn_uintptr_vector_is_empty(
      const KHRN_UINTPTR_VECTOR_T *vector)
{
   return (vector->size == 0);
}

/* removes the last element */
static inline void khrn_uintptr_vector_pop_back(KHRN_UINTPTR_VECTOR_T *vector)
{
   if (vector->size > 0)
      vector->size--;
}

#endif /* KHRN_UINTPTR_VECTOR */
