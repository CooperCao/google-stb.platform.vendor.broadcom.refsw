/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef KHRN_UINTPTR_VECTOR_H
#define KHRN_UINTPTR_VECTOR_H

#include "vcos.h"
#include <stdbool.h>

typedef struct khrn_uintptr_vector
{
   uintptr_t *data;
   size_t capacity;   /* number of allocated elements */
   size_t size;       /* number of used elements */
}
khrn_uintptr_vector;

extern void khrn_uintptr_vector_init(khrn_uintptr_vector *vector);

extern void khrn_uintptr_vector_destroy(khrn_uintptr_vector *vector);

/* inserts a new element at the end */
extern bool khrn_uintptr_vector_push_back(khrn_uintptr_vector *vector,
      uintptr_t elem);

/* return the number of used entries un the vector */
static inline size_t khrn_uintptr_vector_get_size(
      const khrn_uintptr_vector *vector)
{
   return vector->size;
}

/* returns element at position i; it is the users reasponsability to check that i is < vector's size */
static inline uintptr_t khrn_uintptr_vector_item(
      const khrn_uintptr_vector *vector, size_t i)
{
   assert(i < vector->size);
   return vector->data[i];
}

/* erases all the elements */
static inline void khrn_uintptr_vector_clear(khrn_uintptr_vector *vector)
{
   vector->size = 0;
}

/*  returns true if vector size is zero */
static inline bool khrn_uintptr_vector_is_empty(
      const khrn_uintptr_vector *vector)
{
   return (vector->size == 0);
}

/* removes the last element */
static inline void khrn_uintptr_vector_pop_back(khrn_uintptr_vector *vector)
{
   if (vector->size > 0)
      vector->size--;
}

#endif /* KHRN_UINTPTR_VECTOR */
