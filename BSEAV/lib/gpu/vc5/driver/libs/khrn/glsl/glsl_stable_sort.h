/******************************************************************************
*  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
******************************************************************************/
#include <stddef.h>
#include <stdbool.h>
#include "libs/util/common.h"

EXTERN_C_BEGIN

void glsl_stable_sort(const void** base, size_t num,
   bool (*less)(const void*,const void*));

EXTERN_C_END