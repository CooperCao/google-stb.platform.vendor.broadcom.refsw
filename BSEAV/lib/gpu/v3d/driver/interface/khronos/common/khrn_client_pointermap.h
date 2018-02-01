/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include <stdint.h>

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