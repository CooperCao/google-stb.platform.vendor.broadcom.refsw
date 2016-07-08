/*=============================================================================
Broadcom Proprietary and Confidential. (c)2008 Broadcom.
All rights reserved.

Project  :  khronos
Module   :  VG ramp

FILE DESCRIPTION
VG ramp allocation and generation interface.
=============================================================================*/

#ifndef VG_RAMP_H
#define VG_RAMP_H

#include "middleware/khronos/common/khrn_mem.h"
#include "interface/khronos/include/VG/openvg.h"
#include "middleware/khronos/vg/2708/vg_ramp_filler_4.h" /* should #define VG_RAMP_ALIGN */


#define VG_RAMP_HEIGHT 4 /* utile height, so no wasted space with lineartile */
#define VG_RAMP_WIDTH 256
#define VG_RAMP_SIZE (VG_RAMP_HEIGHT * VG_RAMP_WIDTH * sizeof(uint32_t))
#define VG_RAMP_FORMAT ABGR_8888_LT

typedef struct {
   uint32_t ref_counts[VG_RAMP_HEIGHT];
   bool valids[VG_RAMP_HEIGHT]; /* slot i is valid iff ((mem_get_size(data) != 0) && valids[i]) */

   MEM_HANDLE_T data;

   MEM_HANDLE_T free_prev;
   MEM_HANDLE_T free_next;
} VG_RAMP_T;

extern bool vg_ramp_alloc(
   MEM_HANDLE_T *handle,
   uint32_t *i);

extern void vg_ramp_acquire(
   MEM_HANDLE_T handle,
   uint32_t i);
extern void vg_ramp_release(
   MEM_HANDLE_T handle,
   uint32_t ref_count_index);

extern uint32_t vg_ramp_get_rgba(
   MEM_HANDLE_T stops_handle,
   bool last); /* else first */

extern bool vg_ramp_retain(
   MEM_HANDLE_T handle,
   uint32_t i,
   bool pre,
   MEM_HANDLE_T stops_handle);
extern void vg_ramp_unretain(
   MEM_HANDLE_T handle);

#endif
