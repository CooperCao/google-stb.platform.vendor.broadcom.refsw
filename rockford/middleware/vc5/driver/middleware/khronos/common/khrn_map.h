/*=============================================================================
Copyright (c) 2010 Broadcom Europe Limited.
All rights reserved.

Project  :  khronos
Module   :  Map

FILE DESCRIPTION
Relocatable uint32_t to KHRN_MEM_HANDLE_T map.
=============================================================================*/

#ifndef KHRN_MAP_H
#define KHRN_MAP_H

#define khrn_generic_map(X) khrn_map_##X
#define KHRN_GENERIC_MAP(X) KHRN_MAP_##X
#define KHRN_GENERIC_MAP_KEY_T uint32_t
#define KHRN_GENERIC_MAP_VALUE_T KHRN_MEM_HANDLE_T
#define KHRN_GENERIC_MAP_RELOCATABLE

#ifdef KHRN_MAP_C
   #include "interface/khronos/common/khrn_int_generic_map.c"
#else
   #include "interface/khronos/common/khrn_int_generic_map.h"
#endif

#undef KHRN_GENERIC_MAP_RELOCATABLE
#undef KHRN_GENERIC_MAP_VALUE_T
#undef KHRN_GENERIC_MAP_KEY_T
#undef KHRN_GENERIC_MAP
#undef khrn_generic_map

#endif
