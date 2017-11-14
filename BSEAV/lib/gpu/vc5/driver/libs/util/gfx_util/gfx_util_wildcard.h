/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include "libs/util/common.h"
#include <stdbool.h>

EXTERN_C_BEGIN

/* * matches 0 or more characters.
 * ? matches exactly one character.
 * Every other character matches only itself. */
extern bool gfx_wildcard_pattern_matches(const char *p, const char *s);

EXTERN_C_END
