/*=============================================================================
Copyright (c) 2010 Broadcom Europe Limited.
All rights reserved.

Project  :  khronos
Module   :  Pointer map

FILE DESCRIPTION
uint32_t to void * map.
=============================================================================*/

#include "interface/khronos/common/khrn_int_common.h"
#include "interface/khronos/common/khrn_client_platform.h"

#define KHRN_GENERIC_MAP_VALUE_NONE NULL
#define KHRN_GENERIC_MAP_VALUE_DELETED ((void *)(uintptr_t)-1)
#define KHRN_GENERIC_MAP_ALLOC khrn_platform_malloc
#define KHRN_GENERIC_MAP_FREE khrn_platform_free

#define CLIENT_POINTER_MAP_C
#include "interface/khronos/common/khrn_client_pointermap.h"
