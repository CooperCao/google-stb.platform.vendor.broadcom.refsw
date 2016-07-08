/*=============================================================================
Broadcom Proprietary and Confidential. (c)2010 Broadcom.
All rights reserved.

Project  :  khronos
Module   :  64-bit map

FILE DESCRIPTION
Relocatable uint64_t to MEM_HANDLE_T map.
=============================================================================*/

#ifndef KHRN_MAP_64_H
#define KHRN_MAP_64_H

#define khrn_generic_map(X) khrn_map_64_##X
#define KHRN_GENERIC_MAP(X) KHRN_MAP_64_##X
#define KHRN_GENERIC_MAP_KEY_T uint64_t
#define KHRN_GENERIC_MAP_VALUE_T MEM_HANDLE_T
#define KHRN_GENERIC_MAP_RELOCATABLE

#ifdef KHRN_MAP_64_C
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
