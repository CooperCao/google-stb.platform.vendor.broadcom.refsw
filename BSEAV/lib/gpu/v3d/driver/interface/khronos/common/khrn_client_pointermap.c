/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "interface/khronos/common/khrn_int_common.h"
#include "interface/khronos/common/khrn_client_platform.h"

#define KHRN_GENERIC_MAP_VALUE_NONE NULL
#define KHRN_GENERIC_MAP_VALUE_DELETED ((void *)(uintptr_t)-1)
#define KHRN_GENERIC_MAP_ALLOC malloc
#define KHRN_GENERIC_MAP_FREE free

#define CLIENT_POINTER_MAP_C
#include "interface/khronos/common/khrn_client_pointermap.h"
