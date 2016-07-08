/*=============================================================================
Broadcom Proprietary and Confidential. (c)2016 Broadcom.
All rights reserved.
=============================================================================*/
#pragma once

#include "vcos_types.h"
#include <stdbool.h>

VCOS_EXTERN_C_BEGIN

/* * matches 0 or more characters.
 * ? matches exactly one character.
 * Every other character matches only itself. */
extern bool gfx_wildcard_pattern_matches(const char *p, const char *s);

VCOS_EXTERN_C_END
