/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include <stdbool.h>

typedef struct glsl_unique_index_queue_s GLSL_UNIQUE_INDEX_QUEUE_T;

GLSL_UNIQUE_INDEX_QUEUE_T *glsl_unique_index_queue_alloc (int size);
void                       glsl_unique_index_queue_add   (GLSL_UNIQUE_INDEX_QUEUE_T *uiq, int idx);
int                        glsl_unique_index_queue_remove(GLSL_UNIQUE_INDEX_QUEUE_T *uiq);
void                       glsl_unique_index_queue_reset (GLSL_UNIQUE_INDEX_QUEUE_T *uiq);
bool                       glsl_unique_index_queue_empty (GLSL_UNIQUE_INDEX_QUEUE_T *uiq);
