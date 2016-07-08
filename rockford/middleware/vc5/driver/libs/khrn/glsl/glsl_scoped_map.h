/*=============================================================================
Broadcom Proprietary and Confidential. (c)2014 Broadcom.
All rights reserved.

Project  :  glsl
Module   :

FILE DESCRIPTION
=============================================================================*/

#ifndef GLSL_SCOPED_MAP_H
#define GLSL_SCOPED_MAP_H

#include "glsl_map.h"

typedef struct _ScopeList {
   Map* map;
   struct _ScopeList* next;
} ScopeList;

typedef struct _ScopedMap {
   ScopeList* scopes;
} ScopedMap;

// Creates new scoped map.
ScopedMap* glsl_scoped_map_new(void);

// Puts (k,v) into the map.  NULL keys are not allowed.
// Duplicates are allowed and can be accessed by iterating over all members.
void glsl_scoped_map_put(ScopedMap* map, const void* k, void* v);

// Finds k in the map, and then gets the v associated with it.
// If there are duplicates, the most recent value is returend.
void* glsl_scoped_map_get(const ScopedMap* map, const void* k);

// Push nested scope.  All new keys will be added to this scope.
void glsl_scoped_map_push_scope(ScopedMap* map);

// Pop the most recent scope.
void glsl_scoped_map_pop_scope(ScopedMap* map);

#endif // GLSL_SCOPED_MAP_H
