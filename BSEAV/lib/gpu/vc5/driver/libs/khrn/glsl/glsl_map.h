/*=============================================================================
Broadcom Proprietary and Confidential. (c)2014 Broadcom.
All rights reserved.

Project  :  glsl
Module   :

FILE DESCRIPTION
=============================================================================*/

#ifndef GLSL_MAP_H
#define GLSL_MAP_H

#include "vcos.h"

VCOS_EXTERN_C_BEGIN

typedef struct _MapNode
{
   const void* k;
   void* v;
   struct _MapNode* next;
   struct _MapNode* prev;
   struct _MapNode* hashmap_next; // colliding nodes in hashmap
} MapNode;

typedef struct _Map
{
   MapNode* head;
   MapNode* tail;
   int count;
   MapNode** hashmap;
   int num_bits;
} Map;

// Creates new map.
Map* glsl_map_new(void);

// Puts (k,v) into the map.  NULL keys are not allowed.
// Duplicates are allowed and can be accessed by iterating over all members.
void glsl_map_put(Map* map, const void* k, void* v);

// Finds k in the map, and then gets the v associated with it.
// If there are duplicates, the most recent value is returend.
void* glsl_map_get(const Map* map, const void* k);

VCOS_EXTERN_C_END

#endif // MAP_H
