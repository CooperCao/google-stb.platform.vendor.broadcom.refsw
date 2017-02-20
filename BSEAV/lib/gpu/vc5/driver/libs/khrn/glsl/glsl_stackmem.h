/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include <stdlib.h>

void *glsl_stack_malloc (size_t element_size, size_t element_count);
void  glsl_stack_free   (void *v);
int   glsl_stack_cleanup();
