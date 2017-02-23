/*=============================================================================
Broadcom Proprietary and Confidential. (c)2008 Broadcom.
All rights reserved.

Project  :  khronos
Module   :  Vector

FILE DESCRIPTION
Like std::vector, but more C-ey.
=============================================================================*/

#ifndef KHRN_CLIENT_VECTOR_H
#define KHRN_CLIENT_VECTOR_H

typedef struct {
   void *data;
   uint32_t capacity;
   uint32_t size;
} KHRN_VECTOR_T;

extern void khrn_vector_init(KHRN_VECTOR_T *vector, uint32_t capacity);
extern void khrn_vector_term(KHRN_VECTOR_T *vector);

extern bool khrn_vector_extend(KHRN_VECTOR_T *vector, uint32_t size);
extern void khrn_vector_clear(KHRN_VECTOR_T *vector);

#endif
