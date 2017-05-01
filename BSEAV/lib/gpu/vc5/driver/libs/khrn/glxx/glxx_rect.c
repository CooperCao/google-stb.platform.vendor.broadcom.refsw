/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/

#include "glxx_rect.h"

#include "libs/util/gfx_util/gfx_util.h"
#include <limits.h>

static int sat_add(int a, unsigned b)
{
   int res = a + b;
   if (res < a)
      res = INT_MAX;
   return res;
}

void glxx_rect_intersect(glxx_rect *a, const glxx_rect *b)
{
   int xmax = gfx_smin(sat_add(a->x, a->width), sat_add(b->x, b->width));
   int ymax = gfx_smin(sat_add(a->y, a->height), sat_add(b->y, b->height));
   a->x = gfx_smax(a->x, b->x);
   a->y = gfx_smax(a->y, b->y);
   if ((a->x >= xmax) || (a->y >= ymax))
   {
      // Canonical empty rect
      a->x = a->y = 0;
      a->width = a->height = 0;
   }
   else
   {
      a->width = xmax - a->x;
      a->height = ymax - a->y;
   }
}
