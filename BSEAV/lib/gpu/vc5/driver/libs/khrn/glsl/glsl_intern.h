/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include "libs/util/common.h"

EXTERN_C_BEGIN

extern void glsl_init_intern(int size);

extern const char *glsl_intern(const char *s, bool dup);

EXTERN_C_END
