/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include <stddef.h>
#include <stdbool.h>

typedef struct glsl_alloc_tracker_s glsl_alloc_tracker_t;

glsl_alloc_tracker_t *glsl_alloc_tracker_create();
bool                  glsl_alloc_tracker_add (glsl_alloc_tracker_t *tracker,
                                              const char           *file,
                                              int                   line,
                                              size_t                bytes);
void glsl_alloc_tracker_print(const glsl_alloc_tracker_t *tracker);
void glsl_alloc_tracker_free (glsl_alloc_tracker_t       *tracker);
