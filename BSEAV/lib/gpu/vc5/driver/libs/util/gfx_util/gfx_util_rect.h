/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include <stdbool.h>
#include "libs/util/common.h"

EXTERN_C_BEGIN

typedef struct
{
   int x;
   int y;
   unsigned width;
   unsigned height;
} gfx_rect;

extern void gfx_rect_intersect(gfx_rect *a, const gfx_rect *b);
extern void gfx_rect_union(gfx_rect *a, const gfx_rect *b);
extern bool gfx_do_rects_intersect(const gfx_rect *a, const gfx_rect *b);

EXTERN_C_END
