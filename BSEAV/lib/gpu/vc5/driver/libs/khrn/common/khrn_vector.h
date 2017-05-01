/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once
#include "libs/util/common.h"
#include <stddef.h>
#include <stdbool.h>
#include <malloc.h>
#include <assert.h>

// memset to init.
typedef struct khrn_vector
{
   void *data;
   size_t capacity;   /* number of allocated elements */
   size_t size;       /* number of used elements */
}
khrn_vector;

#define khrn_vector_data(type, vec) ((type*)(vec)->data)

#define khrn_vector_emplace_back(type, vec)\
   ((vec)->size < (vec)->capacity || khrn_vector_grow(vec, sizeof(type)) ? khrn_vector_data(type, vec) + (vec)->size++ : NULL)

#define khrn_vector_push_back(type, vec, item)\
   ((vec)->size < (vec)->capacity || khrn_vector_grow(vec, sizeof(item)) ? *(khrn_vector_data(type, vec) + (vec)->size++) = item, true : false)

/* removes the last element */
static inline void khrn_vector_pop_back(khrn_vector *vector)
{
   assert(vector->size > 0);
   vector->size--;
}

/* erases all the elements */
static inline void khrn_vector_clear(khrn_vector *vector)
{
   vector->size = 0;
}

// private.
EXTERN_C_BEGIN
bool khrn_vector_grow(khrn_vector *vector, size_t item_size);
EXTERN_C_END
