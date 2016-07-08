//#define IMAGE_VC_IMAGE /* KHRN_IMAGE_T contains a VC_IMAGE_T (for debugging) */

/*=============================================================================
Broadcom Proprietary and Confidential. (c)2008 Broadcom.
All rights reserved.

Project  :  khronos
Module   :  Image

FILE DESCRIPTION
Khronos image wrapper declarations.
=============================================================================*/

#ifndef KHRN_IMAGE_H
#define KHRN_IMAGE_H

#include "interface/khronos/common/khrn_int_image.h"
#include "middleware/khronos/common/khrn_interlock.h"
#include "middleware/khronos/common/khrn_mem.h"
#include "interface/khronos/include/EGL/egl.h"

#include <string.h>

/******************************************************************************
image handling
******************************************************************************/

/*
   Binding flags
   These specify which, if any, of the 3 EGL binding mechanisms this image is
   taking part in. This information is stored in the image itself rather than
   the API objects which use it, to enable communication between objects.
   (For example if a pbuffer is bound to a texture, releasing the texture means
   the pbuffer can now be bound to a different texture).
   The IMAGE_FLAG_BOUND_CLIENTBUFFER and IMAGE_FLAG_BOUND_TEXIMAGE flags may be
   used together, but not with IMAGE_FLAG_BOUND_EGLIMAGE (giving a total of 5
   possible states).

   texture is responsible for setting and clearing IMAGE_FLAG_BOUND_TEXIMAGE.
   EGL is responsible for setting and clearing IMAGE_FLAG_BOUND_CLIENTBUFFER.
   EGL is responsible for setting IMAGE_FLAG_BOUND_EGLIMAGE (this flag never
      gets cleared once set).
*/
#define IMAGE_FLAG_BOUND_CLIENTBUFFER (1<<0)
#define IMAGE_FLAG_BOUND_TEXIMAGE (1<<1)
#define IMAGE_FLAG_BOUND_EGLIMAGE (1<<2)

/* if these flags are set, the image should be usable in that way (eg if the
 * texture flag is set it should be ok to use this image as a texture). the
 * opposite may not be true. khrn_image_create_dup preserves these flags (so if
 * you duplicate an image that can be used as a texture, the new image will also
 * be usable as a texture) */
#define IMAGE_FLAG_RENDER_TARGET (1<<3)
#define IMAGE_FLAG_TEXTURE       (1<<4)
#define IMAGE_FLAG_RSO_TEXTURE   (1<<5)
#define IMAGE_FLAG_DISPLAY       (1<<6)

typedef struct {
   KHRN_IMAGE_FORMAT_T format;

   uint16_t width;
   uint16_t height;

   int32_t stride; /* in bytes */

   MEM_HANDLE_T mh_palette;
   MEM_HANDLE_T mh_storage;
   uint32_t offset;

   uint16_t flags;

   KHRN_INTERLOCK_T interlock;

   void              *opaque_buffer_handle;
   BEGL_WindowState  *window_state;
   void              *display_thread_state;
   uint32_t          alreadyLocked;
   uint32_t          swap_interval;

   int               fence;
   bool              secure;

} KHRN_IMAGE_T;

typedef enum {
   /*
      select whether the image data should be uninitialized, or initialized to 0xff or 0x00
   */

   IMAGE_CREATE_FLAG_NONE = 0 << 0,
   IMAGE_CREATE_FLAG_ONE  = 1 << 0,
   IMAGE_CREATE_FLAG_ZERO = 2 << 0,

   /*
      select whether the image data should be padded
   */

   IMAGE_CREATE_FLAG_PAD_ROTATE = 1 << 2,

   /*
      usage bits. image parameters will be fudged to ensure the created image
      can be used in the specified manner (assuming the format is acceptable).
      this might involve eg changing the memory layout to t-format or forcing a
      large alignment. once created, the corresponding image flags will be set
      on the image (eg if you pass IMAGE_CREATE_FLAG_TEXTURE to
      khrn_image_create, the created image will have the IMAGE_FLAG_TEXTURE flag
      set)
   */

   IMAGE_CREATE_FLAG_TEXTURE       = 1 << 3,
   IMAGE_CREATE_FLAG_RSO_TEXTURE   = 1 << 4,
   IMAGE_CREATE_FLAG_RENDER_TARGET = 1 << 5,
   IMAGE_CREATE_FLAG_DISPLAY       = 1 << 6,

   /*
      don't allocate storage for the image yet?
   */

   IMAGE_CREATE_FLAG_NO_STORAGE = 1 << 7,

   /*
      mark the image as invalid (khrn_interlock_invalidate)?
   */

   IMAGE_CREATE_FLAG_INVALID = 1 << 8
} KHRN_IMAGE_CREATE_FLAG_T;

#define IMAGE_CREATE_FLAG_INIT_MASK (3 << 0)

extern bool khrn_image_prefer_lt(KHRN_IMAGE_FORMAT_T format, uint32_t width, uint32_t height);
extern uint32_t khrn_image_get_width_ut(const KHRN_IMAGE_T *image);
extern uint32_t khrn_image_get_width_tiles(const KHRN_IMAGE_T *image);
extern uint32_t khrn_image_get_aux_width_ut(const KHRN_IMAGE_T *image);
extern uint32_t khrn_image_wrap_get_width_ut(const KHRN_IMAGE_WRAP_T *wrap);
extern uint32_t khrn_image_wrap_get_width_tiles(const KHRN_IMAGE_WRAP_T *wrap);
extern uint32_t khrn_image_get_align(const KHRN_IMAGE_T *image);
extern uint32_t khrn_image_get_space(const KHRN_IMAGE_T *image);

/* these should only be called on color formats */
extern bool khrn_image_is_ok_for_render_target(KHRN_IMAGE_FORMAT_T format, bool ignore_mem_layout);
extern bool khrn_image_can_use_as_render_target(KHRN_IMAGE_T *image); /* should only be called if khrn_image_is_ok_for_render_target() */

extern void khrn_image_platform_fudge(
   KHRN_IMAGE_FORMAT_T *format,
   uint32_t *padded_width, uint32_t *padded_height,
   uint32_t *align, KHRN_IMAGE_CREATE_FLAG_T flags);

extern void khrn_image_term(void *v, uint32_t);

extern MEM_HANDLE_T khrn_image_create_from_storage(KHRN_IMAGE_FORMAT_T format,
   uint32_t width, uint32_t height, int32_t stride,
   MEM_HANDLE_T palette_handle, MEM_HANDLE_T storage_handle, uint32_t offset,
   KHRN_IMAGE_CREATE_FLAG_T flags, bool secure); /* just used for setting up usage flags */
extern MEM_HANDLE_T khrn_image_create(KHRN_IMAGE_FORMAT_T format,
   uint32_t width, uint32_t height,
   KHRN_IMAGE_CREATE_FLAG_T flags, bool secure);
extern MEM_HANDLE_T khrn_image_create_dup(const KHRN_IMAGE_T *src,
   KHRN_IMAGE_CREATE_FLAG_T flags); /* flags are in addition to implicit flags from src */

extern bool khrn_image_resize(KHRN_IMAGE_T *image, uint32_t width, uint32_t height);

static INLINE void *khrn_image_lock(const KHRN_IMAGE_T *image)
{
   return (uint8_t *)mem_lock(image->mh_storage, NULL) + image->offset;
}

static INLINE void khrn_image_unlock(const KHRN_IMAGE_T *image)
{
   mem_unlock(image->mh_storage);
}

extern void khrn_image_unlock_wrap(const KHRN_IMAGE_T *image);
extern void khrn_image_lock_interlock_wrap(KHRN_IMAGE_T *image, KHRN_IMAGE_WRAP_T *wrap, KHRN_INTERLOCK_T *interlock);

/******************************************************************************
blitting etc
******************************************************************************/

typedef enum {
   /*
      alpha-only images are implicitly black
      to convert rgb to luminance, just take the red channel
      ignore premultiplied and linear flags
   */

   IMAGE_CONV_GL,

   /*
      alpha-only images are implicitly white
      to convert rgb to luminance, take a weighted average
      observe premultiplied and linear flags
   */

   IMAGE_CONV_VG
} KHRN_IMAGE_CONV_T;

extern uint32_t khrn_image_wrap_get_pixel(const KHRN_IMAGE_WRAP_T *wrap, uint32_t x, uint32_t y);
extern void khrn_image_wrap_put_pixel(KHRN_IMAGE_WRAP_T *wrap, uint32_t x, uint32_t y, uint32_t pixel);
extern void khrn_image_wrap_put_etc1_block(KHRN_IMAGE_WRAP_T *wrap, uint32_t x, uint32_t y, uint32_t word0, uint32_t word1);

extern uint32_t khrn_image_pixel_to_rgba(KHRN_IMAGE_FORMAT_T format, uint32_t pixel, KHRN_IMAGE_CONV_T conv);
extern uint32_t khrn_image_rgba_to_pixel(KHRN_IMAGE_FORMAT_T format, uint32_t rgba, KHRN_IMAGE_CONV_T conv);

extern uint32_t khrn_image_rgba_convert_pre_lin(KHRN_IMAGE_FORMAT_T dst_format, KHRN_IMAGE_FORMAT_T src_format, uint32_t rgba);
extern uint32_t khrn_image_rgba_convert_l_pre_lin(KHRN_IMAGE_FORMAT_T dst_format, KHRN_IMAGE_FORMAT_T src_format, uint32_t rgba);

extern void khrn_image_wrap_clear_region(
   KHRN_IMAGE_WRAP_T *wrap, uint32_t x, uint32_t y,
   uint32_t width, uint32_t height,
   uint32_t rgba, /* rgba non-lin, unpre */
   KHRN_IMAGE_CONV_T conv);
extern void khrn_image_wrap_copy_region(
   KHRN_IMAGE_WRAP_T *dst, uint32_t dst_x, uint32_t dst_y,
   uint32_t width, uint32_t height,
   const KHRN_IMAGE_WRAP_T *src, uint32_t src_x, uint32_t src_y,
   KHRN_IMAGE_CONV_T conv);
extern void khrn_image_wrap_copy_scissor_regions(
   KHRN_IMAGE_WRAP_T *dst, uint32_t dst_x, uint32_t dst_y,
   uint32_t width, uint32_t height,
   const KHRN_IMAGE_WRAP_T *src, uint32_t src_x, uint32_t src_y,
   KHRN_IMAGE_CONV_T conv,
   const int32_t *scissor_rects, uint32_t scissor_rects_count);
extern void khrn_image_wrap_convert(KHRN_IMAGE_WRAP_T *dst, const KHRN_IMAGE_WRAP_T *src, KHRN_IMAGE_CONV_T conv);
extern void khrn_image_wrap_copy_stencil_channel(KHRN_IMAGE_WRAP_T *dst, const KHRN_IMAGE_WRAP_T *src);
extern void khrn_image_wrap_subsample(KHRN_IMAGE_WRAP_T *dst, const KHRN_IMAGE_WRAP_T *src);
/*
   KHRN_IMAGE_T functions (forward to the KHRN_IMAGE_WRAP_T functions above)
*/

extern void khrn_image_clear_region(
   KHRN_IMAGE_T *image, uint32_t x, uint32_t y,
   uint32_t width, uint32_t height,
   uint32_t rgba, /* non-lin, unpre */
   KHRN_IMAGE_CONV_T conv);
extern void khrn_image_copy_region(
   KHRN_IMAGE_T *dst, uint32_t dst_x, uint32_t dst_y,
   uint32_t width, uint32_t height,
   const KHRN_IMAGE_T *src, uint32_t src_x, uint32_t src_y,
   KHRN_IMAGE_CONV_T conv);
extern void khrn_image_convert(KHRN_IMAGE_T *dst, const KHRN_IMAGE_T *src, KHRN_IMAGE_CONV_T conv);
extern void khrn_image_copy_stencil_channel(KHRN_IMAGE_T *dst, const KHRN_IMAGE_T *src);
extern void khrn_image_subsample(KHRN_IMAGE_T *dst, const KHRN_IMAGE_T *src);

extern bool khrn_image_alloc_storage(KHRN_IMAGE_T *image, const char *description);

#endif
