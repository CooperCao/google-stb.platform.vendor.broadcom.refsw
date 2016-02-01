/*=============================================================================
Copyright (c) 2010 Broadcom Europe Limited.
All rights reserved.

Project  :  khronos
Module   :  Map

FILE DESCRIPTION
Relocatable uint32_t to KHRN_MEM_HANDLE_T map.
=============================================================================*/

#include "interface/khronos/common/khrn_int_common.h"

#define KHRN_GENERIC_MAP_VALUE_NONE KHRN_MEM_HANDLE_INVALID
#define KHRN_GENERIC_MAP_VALUE_DELETED KHRN_MEM_HANDLE_UNUSED_VALUE
#define KHRN_GENERIC_MAP_ACQUIRE_VALUE khrn_mem_acquire
#define KHRN_GENERIC_MAP_RELEASE_VALUE khrn_mem_release

#define KHRN_MAP_C
#include "middleware/khronos/common/khrn_map.h"
