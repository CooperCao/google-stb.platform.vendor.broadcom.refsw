/*=============================================================================
Broadcom Proprietary and Confidential. (c)2008 Broadcom.
All rights reserved.

Project  :  khronos
Module   :  Fixed-size leaky map

FILE DESCRIPTION
Interface.
=============================================================================*/

#ifndef KHRN_FLEAKY_MAP_H
#define KHRN_FLEAKY_MAP_H

#include "middleware/khronos/common/khrn_mem.h"

typedef struct {
   uint32_t key;
   MEM_HANDLE_T value;
} KHRN_FLEAKY_MAP_ENTRY_T;

typedef struct {
   uint32_t entries;
   uint32_t next_victim;

   KHRN_FLEAKY_MAP_ENTRY_T *storage;
   uint32_t capacity;
} KHRN_FLEAKY_MAP_T;

extern void khrn_fleaky_map_init(KHRN_FLEAKY_MAP_T *fleaky_map, KHRN_FLEAKY_MAP_ENTRY_T *storage, uint32_t capacity);
extern void khrn_fleaky_map_term(KHRN_FLEAKY_MAP_T *fleaky_map);

extern void khrn_fleaky_map_insert(KHRN_FLEAKY_MAP_T *fleaky_map, uint32_t key, MEM_HANDLE_T value); /* mustn't already contain key */
extern MEM_HANDLE_T khrn_fleaky_map_lookup(KHRN_FLEAKY_MAP_T *fleaky_map, uint32_t key);

#endif
