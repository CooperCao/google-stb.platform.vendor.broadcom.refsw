/*=============================================================================
Copyright (c) 2010 Broadcom Europe Limited.
All rights reserved.

Project  :  khronos
Module   :  Global image map

FILE DESCRIPTION
uint32_t to global image map.
=============================================================================*/

#ifndef KHRN_CLIENT_GLOBAL_IMAGE_MAP_H
#define KHRN_CLIENT_GLOBAL_IMAGE_MAP_H

#define khrn_generic_map(X) khrn_global_image_map_##X
#define KHRN_GENERIC_MAP(X) KHRN_GLOBAL_IMAGE_MAP_##X
#define KHRN_GENERIC_MAP_KEY_T uint32_t
#define KHRN_GENERIC_MAP_VALUE_T uint64_t

#ifdef CLIENT_GLOBAL_IMAGE_MAP_C
   #include "interface/khronos/common/khrn_int_generic_map.c"
#else
   #include "interface/khronos/common/khrn_int_generic_map.h"
#endif

#undef KHRN_GENERIC_MAP_VALUE_T
#undef KHRN_GENERIC_MAP_KEY_T
#undef KHRN_GENERIC_MAP
#undef khrn_generic_map

#endif
