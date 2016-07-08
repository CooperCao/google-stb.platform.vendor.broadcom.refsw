/*=============================================================================
Broadcom Proprietary and Confidential. (c)2010 Broadcom.
All rights reserved.

Project  :  khronos
Module   :  Map

FILE DESCRIPTION
Relocatable uint32_t to MEM_HANDLE_T map.
=============================================================================*/

#include "interface/khronos/common/khrn_int_common.h"

#define KHRN_GENERIC_MAP_VALUE_NONE MEM_INVALID_HANDLE
#define KHRN_GENERIC_MAP_VALUE_DELETED ((MEM_HANDLE_T)(MEM_INVALID_HANDLE - 1))
#define KHRN_GENERIC_MAP_ACQUIRE_VALUE mem_acquire
#define KHRN_GENERIC_MAP_RELEASE_VALUE mem_release

#define KHRN_MAP_C
#include "middleware/khronos/common/khrn_map.h"
