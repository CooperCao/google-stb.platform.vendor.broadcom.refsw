/*=============================================================================
Broadcom Proprietary and Confidential. (c)2008 Broadcom.
All rights reserved.

Project  :  khronos
Module   :  VG font

FILE DESCRIPTION
VG font object handling interface.
=============================================================================*/

#ifndef VG_FONT_H
#define VG_FONT_H

#include "middleware/khronos/common/khrn_mem.h"
#include "interface/khronos/include/VG/openvg.h"

typedef struct {
   int32_t glyphs_capacity;
} VG_FONT_BPRINT_T;

extern void vg_font_bprint_from_stem(
   MEM_HANDLE_T handle,
   int32_t glyphs_capacity);

extern void vg_font_bprint_term(MEM_HANDLE_T handle);

static INLINE bool vg_is_font_bprint(MEM_HANDLE_T handle)
{
   return mem_get_term(handle) == vg_font_bprint_term;
}

typedef struct {
   uint32_t id; /* also used for slots free list */

   MEM_HANDLE_T object; /* VG_PATH_T, VG_IMAGE_T, or VG_CHILD_IMAGE_T */
   bool allow_autohinting;
   float origin[2];
   float escapement[2];
} VG_GLYPH_T;

#define SLOT_NONE ((uint32_t)-1)

typedef struct {
   uint32_t capacity;
   uint32_t count;
   MEM_HANDLE_T entries; /* mem_get_size() == (capacity * 2) * sizeof(uint32_t) */
   uint32_t slots_free_head;
   MEM_HANDLE_T slots; /* mem_get_size() == capacity * sizeof(VG_GLYPH_T) */
} VG_FONT_T;

extern bool vg_font_from_bprint(MEM_HANDLE_T handle);

extern void vg_font_term(MEM_HANDLE_T handle);

static INLINE bool vg_is_font(MEM_HANDLE_T handle)
{
   return mem_get_term(handle) == vg_font_term;
}

extern bool vg_font_insert(
   VG_FONT_T *font,
   uint32_t glyph_id,
   MEM_HANDLE_T object_handle,
   bool allow_autohinting,
   float origin_x, float origin_y,
   float escapement_x, float escapement_y);

extern bool vg_font_delete(
   VG_FONT_T *font,
   uint32_t glyph_id);

typedef struct {
   uint32_t capacity;
   uint32_t *entries;
   VG_GLYPH_T *slots;
} VG_FONT_LOCKED_T;

extern void vg_font_lock(
   VG_FONT_T *font,
   VG_FONT_LOCKED_T *font_locked);
extern void vg_font_unlock(
   VG_FONT_T *font);

extern VG_GLYPH_T *vg_font_locked_lookup(
   VG_FONT_LOCKED_T *font_locked,
   uint32_t glyph_id);

#endif
