/*=============================================================================
Broadcom Proprietary and Confidential. (c)2008 Broadcom.
All rights reserved.

Project  :  khronos
Module   :  VG set

FILE DESCRIPTION
Set of memory handles.
=============================================================================*/

#ifndef VG_SET_H
#define VG_SET_H

/*
   todo: this is just a wrapper over map for the moment
*/

#include "middleware/khronos/common/khrn_map.h"

typedef KHRN_MAP_T VG_SET_T;

extern bool vg_set_init(VG_SET_T *set, uint32_t capacity);

extern bool vg_set_insert(VG_SET_T *set, MEM_HANDLE_T key);
extern void vg_set_delete(VG_SET_T *set, MEM_HANDLE_T key);

extern bool vg_set_contains(VG_SET_T *set, MEM_HANDLE_T key);
extern bool vg_set_contains_locked(VG_SET_T *set, MEM_HANDLE_T key, void *storage);

typedef void (*SET_CALLBACK_T)(VG_SET_T *set, MEM_HANDLE_T key, void *);
extern void vg_set_iterate(VG_SET_T *set, SET_CALLBACK_T func, void *);

extern void vg_set_term(VG_SET_T *set);

#endif
