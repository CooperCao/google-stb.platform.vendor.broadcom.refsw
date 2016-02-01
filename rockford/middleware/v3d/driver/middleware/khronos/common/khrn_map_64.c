/*=============================================================================
Copyright (c) 2010 Broadcom Europe Limited.
All rights reserved.

Project  :  khronos
Module   :  64-bit map

FILE DESCRIPTION
Relocatable uint64_t to MEM_HANDLE_T map.
=============================================================================*/

#include "interface/khronos/common/khrn_int_common.h"

#define KHRN_GENERIC_MAP_VALUE_NONE MEM_INVALID_HANDLE
#define KHRN_GENERIC_MAP_VALUE_DELETED ((MEM_HANDLE_T)(MEM_INVALID_HANDLE - 1))
#define KHRN_GENERIC_MAP_ACQUIRE_VALUE mem_acquire
#define KHRN_GENERIC_MAP_RELEASE_VALUE mem_release

#define KHRN_MAP_64_C
#include "middleware/khronos/common/khrn_map_64.h"
