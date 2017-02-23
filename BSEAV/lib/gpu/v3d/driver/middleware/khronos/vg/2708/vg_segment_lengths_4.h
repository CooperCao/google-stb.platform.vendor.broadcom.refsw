/*=============================================================================
Broadcom Proprietary and Confidential. (c)2008 Broadcom.
All rights reserved.

Project  :  khronos
Module   :  VG segment length calculations

FILE DESCRIPTION
VG segment length calculations interface.
=============================================================================*/

#ifndef VG_SEGMENT_LENGTHS_4_H
#define VG_SEGMENT_LENGTHS_4_H

#include "middleware/khronos/vg/vg_segment_lengths.h"

extern float vg_segment_lengths_get_length(
   const uint8_t *segments, uint32_t segments_i, uint32_t segments_count,
   const void *chunks);
extern void vg_segment_lengths_get_p_t_along(
   float *p, float *t,
   const uint8_t *segments, uint32_t segments_i, uint32_t segments_count,
   const void *chunks,
   float distance);

#endif
