/*=============================================================================
Broadcom Proprietary and Confidential. (c)2008 Broadcom.
All rights reserved.

Project  :  khronos
Module   :  VG segment length calculations

FILE DESCRIPTION
Non-platform-specific stuff.
=============================================================================*/

#ifndef VG_SEGMENT_LENGTHS_H
#define VG_SEGMENT_LENGTHS_H

extern float vg_segment_lengths_get_line_length(float p0_x, float p0_y, float p1_x, float p1_y);
extern float vg_segment_lengths_get_cubic_length(const float *p);

extern float vg_segment_lengths_get_t_along_line(float p0_x, float p0_y, float p1_x, float p1_y, float distance);
extern float vg_segment_lengths_get_t_along_cubic(const float *p, float distance);

extern void vg_segment_lengths_get_line_p(float *p, float p0_x, float p0_y, float p1_x, float p1_y, float t);
extern void vg_segment_lengths_get_cubic_p(float *p, const float *q, float t);

extern bool vg_segment_lengths_get_line_t(float *t, float p0_x, float p0_y, float p1_x, float p1_y);
extern bool vg_segment_lengths_get_cubic_t(float *t, const float *p, float u);

#endif
