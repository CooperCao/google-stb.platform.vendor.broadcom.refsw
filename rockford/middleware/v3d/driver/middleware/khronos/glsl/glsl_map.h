/*=============================================================================
Broadcom Proprietary and Confidential. (c)2008 Broadcom.
All rights reserved.

Project  :  khronos
Module   :

FILE DESCRIPTION
Standalone GLSL compiler
=============================================================================*/
#ifndef GLSL_MAP_H
#define GLSL_MAP_H

struct _MapNode;
typedef struct _MapNode MapNode;

struct _MapNode
{
   const void* k;
   void* v;
   MapNode* next;
};

struct _Map
{
   MapNode* head;
};

// Creates new map.
Map* glsl_map_new(void);

// Clears all nodes from the map.
Map* glsl_map_clear(Map* map);

// Puts (k,v) into the map.
// Does *not* guarantee that k is unique in the map.
// User cannot put NULL keys as they are used for checkpoints.
bool glsl_map_put(Map* map, const void* k, void* v);

// Finds the latest occurrence of k in the map, and then gets the v associated with it.
// User cannot get NULL keys as they are used for checkpoints.
void *glsl_map_get(Map* map, const void* k, bool cross_checkpoints);

// Sets a checkpoint.
bool glsl_map_set_checkpoint(Map* map);

// Clears everything up to the last checkpoint.
bool glsl_map_clear_to_checkpoint(Map* map);

// Pops the latest value to be added.
void* glsl_map_pop(Map* map);

//reverses underlying list
void glsl_map_reverse(Map* map);

#endif // MAP_H
