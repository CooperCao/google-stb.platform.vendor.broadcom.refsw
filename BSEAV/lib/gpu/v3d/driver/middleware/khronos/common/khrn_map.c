/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "interface/khronos/common/khrn_int_common.h"

#define KHRN_GENERIC_MAP_VALUE_NONE MEM_HANDLE_INVALID
#define KHRN_GENERIC_MAP_VALUE_DELETED ((MEM_HANDLE_T)(MEM_HANDLE_INVALID - 1))
#define KHRN_GENERIC_MAP_ACQUIRE_VALUE mem_acquire
#define KHRN_GENERIC_MAP_RELEASE_VALUE mem_release

#define KHRN_MAP_C
#include "middleware/khronos/common/khrn_map.h"
