/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef GFX_UTIL_MORTON_H
#define GFX_UTIL_MORTON_H

#include "gfx_util.h"

EXTERN_C_BEGIN

/* See http://en.wikipedia.org/wiki/Morton_order */

/* y\x 0      1      2      3      4      5      6      7
 *
 * 0   0------1      4------5     16-----17     20-----21
 *           /      /      /      /      /      /      /
 *       +--+   +--+   +--+      /   +--+   +--+   +--+
 *      /      /      /         /   /      /      /
 * 1   2------3      6------7   | 18-----19     22-----23
 *                         /    |                      /
 *       +----------------+     |    +----------------+
 *      /                       |   /
 * 2   8------9     12-----13   | 24-----25     28-----29
 *           /      /      /   /         /      /      /
 *       +--+   +--+   +--+   /      +--+   +--+   +--+
 *      /      /      /      /      /      /      /
 * 3  10-----11     14-----15     26-----27     30-----31
 *
 * And so on...
 */
extern void gfx_morton_to_xy(uint32_t *x, uint32_t *y, uint32_t i);
extern uint32_t gfx_morton_from_xy(uint32_t x, uint32_t y);

/** Iterating over a rectangle in Morton order */

/* Transpose is applied before x/y flips. It is also applied before clipping to
 * w/h, so the x you get out will always be < w and the y < h. It essentially
 * just turns the hierarchical zs into hierarchical |/|s */
#define GFX_MORTON_TRANSPOSE  (1 << 0)

/* Flip returned x/y coordinates */
#define GFX_MORTON_FLIP_X     (1 << 1)
#define GFX_MORTON_FLIP_Y     (1 << 2)

/* Return coordinates in reverse order. This is *not* the same thing as setting
 * both FLIP_X and FLIP_Y */
#define GFX_MORTON_REVERSE    (1 << 3)

typedef struct {
   uint32_t w, h;
   uint32_t flags;
   uint32_t i, inc_i, end_i;
} GFX_MORTON_STATE_T;

extern void gfx_morton_init(GFX_MORTON_STATE_T *state,
   uint32_t w, uint32_t h, uint32_t flags);

/* Returns false if there are no more coordinates. true is returned up to and
 * including the last one */
extern bool gfx_morton_next(GFX_MORTON_STATE_T *state,
   uint32_t *x, uint32_t *y, bool *last);

EXTERN_C_END

#endif
