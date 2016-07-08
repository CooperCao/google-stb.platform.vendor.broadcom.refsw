/*=============================================================================
Broadcom Proprietary and Confidential. (c)2008 Broadcom.
All rights reserved.

Project  :  khronos
Module   :  VG scissor

FILE DESCRIPTION
VG scissor allocation and generation interface.
=============================================================================*/

#ifndef VG_SCISSOR_H
#define VG_SCISSOR_H

#include "middleware/khronos/common/khrn_mem.h"
#include "middleware/khronos/vg/2708/vg_scissor_filler_4.h" /* should #define VG_SCISSOR_ALIGN and define vg_scissor_get_format */

extern MEM_HANDLE_T vg_scissor_alloc(void);

extern bool vg_scissor_retain(
   MEM_HANDLE_T handle, int32_t *bounds,
   uint32_t width, uint32_t height,
   const int32_t *rects, uint32_t rects_count);

#endif
