/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "glsl_stable_sort.h"
#include <algorithm>

void glsl_stable_sort(const void** base, size_t num,
   bool (*less)(const void*,const void*))
{
   std::stable_sort(base, base + num, less);
}
