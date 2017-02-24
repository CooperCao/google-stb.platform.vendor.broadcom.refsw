/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include <assert.h>
#include "libs/util/common.h"

EXTERN_C_BEGIN

typedef struct _MapEntry
{
   const void *k;
   void *v;
} MapEntry;

typedef struct _Map
{
   int count; // Number of entries in the map
   int num_bits;
   MapEntry  *entries; // Pointer to an array of size GLSL_MAP_MAX_ENTRIES(num_bits)
   MapEntry **buckets; // Pointer to an array of size (1 << num_bits)
} Map;

// Create/delete maps.
Map *glsl_map_new(void);
void glsl_map_delete(Map *map);

// Puts (k,v) into the map.  NULL keys are not allowed.
// Duplicates are allowed and can be accessed by iterating over all members.
void glsl_map_put(Map *map, const void *k, void *v);

// Finds k in the map, and then gets the v associated with it.
// If there are duplicates, the most recent value is returend.
void *glsl_map_get(const Map *map, const void *k);

// Iteration order matches insertion order
#define GLSL_MAP_FOREACH(ENTRY, MAP) \
   for (MapEntry *ENTRY = (MAP)->entries, *end_ = ENTRY + (MAP)->count; ENTRY != end_; ++ENTRY)

EXTERN_C_END
