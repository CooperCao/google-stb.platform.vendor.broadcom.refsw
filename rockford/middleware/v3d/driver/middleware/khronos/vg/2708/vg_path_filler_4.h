/*=============================================================================
Copyright (c) 2008 Broadcom Europe Limited.
All rights reserved.

Project  :  khronos
Module   :  VG path

FILE DESCRIPTION
Extra platform-specific path stuff. This should only be included by
../vg_path.h.
=============================================================================*/

#ifndef VG_PATH_FILLER_4_H
#define VG_PATH_FILLER_4_H

#include "middleware/khronos/common/khrn_mem.h"

#define VG_PATH_FLAG_BOUNDS_VALID (1 << 4)
#define VG_PATH_FLAG_BOUNDS_LOOSE (1 << 5)
#define VG_PATH_FLAG_INTERP_TO    (1 << 6)
#define VG_PATH_FLAG_INTERP_FROM  (1 << 7)

typedef union {
   struct {
      MEM_HANDLE_T tess;

      union {
         struct {
            void *fill_rep, *stroke_rep;
            uint32_t fill_state_id[2], stroke_state_id[2]; /* the reason these aren't uint64_ts is to avoid alignment issues on platforms where alignof(uint64_t) > 4 */
         } glyph;
         struct {
            MEM_HANDLE_T head_a, head_b; /* don't hold references. paths should remove themselves from the lls in vg_path_extra_term (via discard_interp_to) */
         } interp_from;
      } u;
   } n;
   struct {
      MEM_HANDLE_T from_a, from_b, prev_a, prev_b, next_a, next_b; /* hold references to from_a and from_b */
      float t;
   } interp_to;
} VG_PATH_EXTRA_T;

#endif
