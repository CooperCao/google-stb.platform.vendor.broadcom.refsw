/*=============================================================================
Copyright (c) 2014 Broadcom Europe Limited.
All rights reserved.

Project  :  glsl
Module   :

FILE DESCRIPTION
=============================================================================*/

#ifndef GLSL_STACKMEM_H_INCLUDED
#define GLSL_STACKMEM_H_INCLUDED

#include <stdlib.h>

void *glsl_stack_malloc (size_t element_size, size_t element_count);
void  glsl_stack_free   (void *v);
int   glsl_stack_cleanup();

#endif
