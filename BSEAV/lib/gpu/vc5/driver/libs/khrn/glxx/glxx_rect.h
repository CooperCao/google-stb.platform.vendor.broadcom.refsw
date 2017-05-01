/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

typedef struct
{
   int x;
   int y;
   unsigned width;
   unsigned height;
} glxx_rect;

extern void glxx_rect_intersect(glxx_rect *a, const glxx_rect *b);
