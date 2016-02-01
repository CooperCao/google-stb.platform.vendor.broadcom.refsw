/*=============================================================================
Copyright (c) 2010 Broadcom Europe Limited.
All rights reserved.

Project  :  khronos
Module   :  Pointer map

FILE DESCRIPTION
uint32_t to void * map.
=============================================================================*/

#ifndef KHRN_CLIENT_POINTERMAP_H
#define KHRN_CLIENT_POINTERMAP_H

#define khrn_generic_map(X) khrn_pointer_map_##X
#define KHRN_GENERIC_MAP(X) KHRN_POINTER_MAP_##X
#define KHRN_GENERIC_MAP_KEY_T uint32_t
#define KHRN_GENERIC_MAP_VALUE_T void *

#ifdef CLIENT_POINTER_MAP_C
   #include "interface/khronos/common/khrn_int_generic_map.c"
#else
   #include "interface/khronos/common/khrn_int_generic_map.h"
#endif

#undef KHRN_GENERIC_MAP_VALUE_T
#undef KHRN_GENERIC_MAP_KEY_T
#undef KHRN_GENERIC_MAP
#undef khrn_generic_map

#endif
