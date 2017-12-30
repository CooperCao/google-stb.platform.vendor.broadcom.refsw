/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "interface/khronos/common/khrn_int_common.h"

#include "interface/khronos/common/khrn_int_color.h"
#include "middleware/khronos/common/khrn_hw.h"
#include "middleware/khronos/common/khrn_image.h"
#include "middleware/khronos/common/khrn_tformat.h"
#include "interface/khronos/common/khrn_int_util.h"
#include "middleware/khronos/common/khrn_bf.h"

#include "middleware/khronos/egl/egl_server.h"
#include "middleware/khronos/egl/egl_platform.h"

#include <stdlib.h>
#include <string.h>

#ifdef __ARM_NEON__
#include <arm_neon.h>
#endif

#define DEFAULT_ALIGN 64 /* todo: what should this be? */

#define BYTE_SWAP_W(w) \
{ \
   uint32_t tmp = w;\
   uint8_t *t = (uint8_t*)&tmp;\
   uint8_t *p = (uint8_t*)&w;\
   p[0] = t[3];\
   p[1] = t[2];\
   p[2] = t[1];\
   p[3] = t[0];\
}

/******************************************************************************
helpers
******************************************************************************/

static uint32_t get_width_ut(KHRN_IMAGE_FORMAT_T format, int32_t stride)
{
   assert(stride > 0);
   return ((uint32_t)(stride << 3) / khrn_image_get_bpp(format)) >> khrn_image_get_log2_utile_width(format);
}

/******************************************************************************
image handling
******************************************************************************/

bool khrn_image_prefer_lt(KHRN_IMAGE_FORMAT_T format, uint32_t width, uint32_t height)
{
   /* this is not arbitrary -- it matches the rule by which the 2708 hw chooses between lt/tf
    * todo: which means it should probably be platform-specific */
   return (width <= (uint32_t)(4 << khrn_image_get_log2_utile_width(format))) ||
          (height <= (uint32_t)(4 << khrn_image_get_log2_utile_height(format)));
}

uint32_t khrn_image_get_width_ut(const KHRN_IMAGE_T *image)
{
   return get_width_ut(image->format, image->stride);
}

uint32_t khrn_image_get_width_tiles(const KHRN_IMAGE_T *image)
{
   return get_width_ut(image->format, image->stride) >> 3;
}

uint32_t khrn_image_get_aux_width_ut(const KHRN_IMAGE_T *image)
{
   switch (image->format) {
   case DEPTH_16_TF:
   case DEPTH_32_TF:
   {
      uint32_t early_z_width = (image->width + 3) >> 2;
      uint32_t early_z_stride = khrn_image_get_stride(DEPTH_16_TF, early_z_width);
      return get_width_ut(DEPTH_16_TF, early_z_stride);
   }
   default:  /* This function only makes sense for early z auxiliary image */
      UNREACHABLE();
      return 0;
   }
}

uint32_t khrn_image_wrap_get_width_ut(const KHRN_IMAGE_WRAP_T *wrap)
{
   return get_width_ut(wrap->format, wrap->stride);
}

uint32_t khrn_image_wrap_get_width_tiles(const KHRN_IMAGE_WRAP_T *wrap)
{
   return get_width_ut(wrap->format, wrap->stride) >> 3;
}

uint32_t khrn_image_get_align(const KHRN_IMAGE_T *image)
{
   return image->offset ? (uint32_t)_min(mem_get_align(image->mh_storage),
      image->offset & ~(image->offset - 1)) :
      mem_get_align(image->mh_storage);
}

uint32_t khrn_image_get_space(const KHRN_IMAGE_T *image)
{
   return mem_get_size(image->mh_storage) - image->offset;
}

void khrn_image_term(MEM_HANDLE_T handle)
{
   KHRN_IMAGE_T *image = (KHRN_IMAGE_T *)mem_lock(handle, NULL);

   khrn_interlock_term(&image->interlock);

   MEM_ASSIGN(image->mh_storage, MEM_HANDLE_INVALID);
   MEM_ASSIGN(image->mh_palette, MEM_HANDLE_INVALID);

   mem_unlock(handle);
}

MEM_HANDLE_T khrn_image_create_from_storage(KHRN_IMAGE_FORMAT_T format,
   uint32_t width, uint32_t height, int32_t stride,
   MEM_HANDLE_T palette_handle, MEM_HANDLE_T storage_handle, uint32_t offset,
   KHRN_IMAGE_CREATE_FLAG_T flags, bool secure)
{
   MEM_HANDLE_T handle;
   KHRN_IMAGE_T *image;

   assert(format != IMAGE_FORMAT_INVALID);

   /*
      alloc struct
   */

   handle = MEM_ALLOC_STRUCT_EX(KHRN_IMAGE_T, MEM_COMPACT_DISCARD);
   if (handle == MEM_HANDLE_INVALID) {
      return MEM_HANDLE_INVALID;
   }

   /*
      fill in the struct
   */

   image = (KHRN_IMAGE_T *)mem_lock(handle, NULL);
   image->format = format;
   image->width = (uint16_t)width;
   image->height = (uint16_t)height;
   image->stride = stride;
   if (palette_handle != MEM_HANDLE_INVALID) {
      mem_acquire(palette_handle);
   }
   image->mh_palette = palette_handle;
   if (storage_handle != MEM_HANDLE_INVALID) {
      mem_acquire(storage_handle);
   }
   image->mh_storage = storage_handle;
   image->offset = offset;
   image->flags =
      ((flags & IMAGE_CREATE_FLAG_TEXTURE) ? IMAGE_FLAG_TEXTURE : 0) |
      ((flags & IMAGE_CREATE_FLAG_RSO_TEXTURE) ? IMAGE_FLAG_RSO_TEXTURE : 0) |
      ((flags & IMAGE_CREATE_FLAG_RENDER_TARGET) ? IMAGE_FLAG_RENDER_TARGET : 0) |
      ((flags & IMAGE_CREATE_FLAG_DISPLAY) ? IMAGE_FLAG_DISPLAY : 0);
   image->secure = secure;
   khrn_interlock_init(&image->interlock);
   if (flags & IMAGE_CREATE_FLAG_INVALID) {
      khrn_interlock_invalidate(&image->interlock);
   }
   image->v3dfence = NULL;
   image->platform_pixmap = NULL;

   mem_unlock(handle);

   /*
      set the terminator
   */

   mem_set_term(handle, khrn_image_term, NULL);

   return handle;
}

MEM_HANDLE_T khrn_image_create(KHRN_IMAGE_FORMAT_T format,
   uint32_t width, uint32_t height,
   KHRN_IMAGE_CREATE_FLAG_T flags,
   bool secure)
{
   uint32_t padded_width = width, padded_height = height, align = DEFAULT_ALIGN, storage_size;
   MEM_HANDLE_T palette_handle, storage_handle, handle;

   assert(format != IMAGE_FORMAT_INVALID);

   khrn_image_platform_fudge(&format, &padded_width, &padded_height, &align, flags);

   /*
      alloc palette / early z if needed
   */

   palette_handle = MEM_HANDLE_INVALID;

   if (khrn_image_is_paletted(format)) {
      palette_handle = mem_alloc_ex((1 << khrn_image_get_bpp(format)) * sizeof(uint32_t),
         alignof(uint32_t), MEM_FLAG_NONE, "KHRN_IMAGE_T.palette", MEM_COMPACT_DISCARD);      // check, no term
      if (palette_handle == MEM_HANDLE_INVALID) {
         return MEM_HANDLE_INVALID;
      }
   }

   /*
      alloc storage
   */

   if (flags & IMAGE_CREATE_FLAG_NO_STORAGE) {
      storage_handle = MEM_HANDLE_INVALID;
   } else {
      storage_size = khrn_image_get_size(format, padded_width, padded_height);
      if (flags & IMAGE_CREATE_FLAG_PAD_ROTATE)
         storage_size = _max(storage_size, khrn_image_get_size(format, padded_height, padded_width));

      MEM_FLAG_T mem_flag =
         MEM_FLAG_DIRECT | MEM_FLAG_256BIT_PAD | MEM_FLAG_RESIZEABLE;
      if ((flags & IMAGE_CREATE_FLAG_INIT_MASK) == IMAGE_CREATE_FLAG_ZERO)
         mem_flag |= MEM_FLAG_ZERO | MEM_FLAG_INIT;
      else
         mem_flag |= MEM_FLAG_NO_INIT;
      if (secure)
         mem_flag |= (MEM_FLAG_SECURE | MEM_FLAG_NO_INIT);

      storage_handle = mem_alloc_ex(storage_size, align,
         mem_flag,
         "KHRN_IMAGE_T.storage",
         MEM_COMPACT_DISCARD);     // check, no term
      if (storage_handle == MEM_HANDLE_INVALID) {
         if (palette_handle != MEM_HANDLE_INVALID)
            mem_release(palette_handle);
         return MEM_HANDLE_INVALID;
      }
      if ((flags & IMAGE_CREATE_FLAG_INIT_MASK) == IMAGE_CREATE_FLAG_ONE) {
         khrn_memset(mem_lock(storage_handle, NULL), -1, storage_size);
         mem_unlock(storage_handle);
      }
   }

   /*
      alloc and fill in struct
   */

   handle = khrn_image_create_from_storage(format,
      width, height, khrn_image_get_stride(format, padded_width),
      palette_handle, storage_handle, 0, flags, secure);
   if (palette_handle != MEM_HANDLE_INVALID) {
      mem_release(palette_handle);
   }
   if (storage_handle != MEM_HANDLE_INVALID) {
      mem_release(storage_handle);
   }

   return handle;
}

MEM_HANDLE_T khrn_image_create_dup(const KHRN_IMAGE_T *src,
   KHRN_IMAGE_CREATE_FLAG_T flags)
{
   MEM_HANDLE_T handle;

   /*
      alloc new image
   */

   KHRN_IMAGE_CREATE_FLAG_T image_create_flags =
      ((src->flags & IMAGE_FLAG_TEXTURE) ? IMAGE_CREATE_FLAG_TEXTURE : 0) |
      ((src->flags & IMAGE_FLAG_RSO_TEXTURE) ? IMAGE_CREATE_FLAG_RSO_TEXTURE : 0) |
      ((src->flags & IMAGE_FLAG_RENDER_TARGET) ? IMAGE_CREATE_FLAG_RENDER_TARGET : 0) |
      ((src->flags & IMAGE_FLAG_DISPLAY) ? IMAGE_CREATE_FLAG_DISPLAY : 0);

   handle = khrn_image_create(src->format, src->width, src->height,
      /* todo: preserve rotate padding? */
      flags | image_create_flags, false);
   if (handle == MEM_HANDLE_INVALID) {
      return MEM_HANDLE_INVALID;
   }

   /*
      copy
   */

   KHRN_IMAGE_T *dst = (KHRN_IMAGE_T *)mem_lock(handle, NULL);
   assert((src->mh_palette == MEM_HANDLE_INVALID) == (dst->mh_palette == MEM_HANDLE_INVALID));
   if (src->mh_palette != MEM_HANDLE_INVALID) {
      assert(mem_get_size(src->mh_palette) == mem_get_size(dst->mh_palette));
      khrn_memcpy(mem_lock(dst->mh_palette, NULL), mem_lock(src->mh_palette, NULL), mem_get_size(src->mh_palette));
      mem_unlock(dst->mh_palette);
      mem_unlock(src->mh_palette);
   }
   khrn_image_convert(dst, src, IMAGE_CONV_GL);
   mem_unlock(handle);

   return handle;
}

bool khrn_image_resize(KHRN_IMAGE_T *image, uint32_t width, uint32_t height)
{
   KHRN_IMAGE_CREATE_FLAG_T image_create_flags =
      ((image->flags & IMAGE_FLAG_TEXTURE) ? IMAGE_CREATE_FLAG_TEXTURE : 0) |
      ((image->flags & IMAGE_FLAG_RSO_TEXTURE) ? IMAGE_CREATE_FLAG_RSO_TEXTURE : 0) |
      ((image->flags & IMAGE_FLAG_RENDER_TARGET) ? IMAGE_CREATE_FLAG_RENDER_TARGET : 0) |
      ((image->flags & IMAGE_FLAG_DISPLAY) ? IMAGE_CREATE_FLAG_DISPLAY : 0);

   uint32_t padded_width = width, padded_height = height, align = DEFAULT_ALIGN;
   KHRN_IMAGE_FORMAT_T format = image->format;
   khrn_image_platform_fudge(&format, &padded_width, &padded_height, &align, image_create_flags);
   assert(format == image->format);
   assert(image->mh_storage == MEM_HANDLE_INVALID || align <= khrn_image_get_align(image));

   if (image->mh_storage != MEM_HANDLE_INVALID && mem_get_size(image->mh_storage) != 0) {
      uint32_t storage_size = khrn_image_get_size(image->format, padded_width, padded_height);
      if (mem_get_size(image->mh_storage) < storage_size) {
         if (!mem_resize_ex(image->mh_storage, storage_size, MEM_COMPACT_DISCARD))
            return false;
      }
   }

   image->width = width;
   image->height = height;
   image->stride = khrn_image_get_stride(image->format, padded_width);

   return true;
}

void khrn_image_lock_wrap(KHRN_IMAGE_T *image, KHRN_IMAGE_WRAP_T *wrap)
{
   wrap->format = image->format;
   wrap->secure = image->secure;
   wrap->width = image->width;
   wrap->height = image->height;
   wrap->stride = image->stride;
   wrap->flags = image->flags;
   wrap->palette = (image->mh_palette == MEM_HANDLE_INVALID) ? NULL : mem_lock(image->mh_palette, NULL);
   wrap->storage = (char*)mem_lock(image->mh_storage, NULL) + image->offset;
}

void khrn_image_unlock_wrap(const KHRN_IMAGE_T *image)
{
   mem_unlock(image->mh_storage);
   if (image->mh_palette != MEM_HANDLE_INVALID) {
      mem_unlock(image->mh_palette);
   }
}

/******************************************************************************
blitting etc
******************************************************************************/

static INLINE uint32_t get_bit(const uint8_t *addr, uint32_t bit)
{
   assert(bit < 8);

   return *addr >> bit & 1;
}

static INLINE uint32_t get_nibble(const uint8_t *addr, uint32_t nibble)
{
   assert(nibble < 2);

   nibble <<= 2;

   return *addr >> nibble & 0xf;
}

uint32_t khrn_image_wrap_get_pixel(const KHRN_IMAGE_WRAP_T *wrap, uint32_t x, uint32_t y)
{
   uint32_t pixel = 0;

   assert(khrn_image_is_uncomp(wrap->format));
   assert(x < wrap->width);
   assert(y < wrap->height);

   if (wrap->storage == NULL)
   {
      ANDROID_LOGD("==========================================================================\n");
      ANDROID_LOGD("khrn_image_wrap_get_pixel() CALLED WITH NULL POINTER FOR SURFACE WRAP!!!!! \n");
      ANDROID_LOGD("==========================================================================\n");
   }

   switch (wrap->format & IMAGE_FORMAT_MEM_LAYOUT_MASK) {
   case IMAGE_FORMAT_RSO:
   {
      /* either the start or end of the image (depending on whether its marked as IMAGE_FLAG_DISPLAY or not.  Pixmaps and Window surfaces should be */
      void *row = (uint8_t *)wrap->storage + (((wrap->flags & IMAGE_FLAG_DISPLAY) ? 1 : 0) * (wrap->stride * (wrap->height - 1)));
      /* egl images traverse down the image, with first pixel in memory being GL's last row */
      row = (uint8_t *)row + (y * (((wrap->flags & IMAGE_FLAG_DISPLAY) ? -1 : 1) * wrap->stride));

      switch (wrap->format & IMAGE_FORMAT_PIXEL_SIZE_MASK) {
      case IMAGE_FORMAT_1:  pixel = get_bit((uint8_t *)row + (x >> 3), x & 0x7); break;
      case IMAGE_FORMAT_4:  pixel = get_nibble((uint8_t *)row + (x >> 1), x & 0x1); break;
      case IMAGE_FORMAT_8:  pixel = ((uint8_t *)row)[x]; break;
      case IMAGE_FORMAT_16:
         assert(!((uintptr_t)row & 1));
         if (khrn_image_is_yuv422(wrap->format))
         {
            if (x & 1)
               pixel = ((uint8_t *)row)[2*x+0] | ((uint8_t *)row)[2*x-1]<<8 | ((uint8_t *)row)[2*x+1]<<16;
            else
               pixel = ((uint8_t *)row)[2*x+0] | ((uint8_t *)row)[2*x+1]<<8 | ((uint8_t *)row)[2*x+3]<<16;
         }
         else
            pixel = ((uint16_t *)row)[x];
         break;
      case IMAGE_FORMAT_32: assert(!((uintptr_t)row & 3));
         pixel = ((uint32_t *)row)[x];
         break;
      case IMAGE_FORMAT_24:
         pixel = ((uint8_t *)row)[3*x+2]<<16 | ((uint8_t *)row)[3*x+1]<<8 | ((uint8_t *)row)[3*x];
         break;
      default:              UNREACHABLE();
      }

      break;
   }
   case IMAGE_FORMAT_TF:
   case IMAGE_FORMAT_LT:
   {
      uint32_t log2_utile_width, log2_utile_height;
      uint32_t ut_w, ut_x, ut_y;
      void *ut_base;

      /* modify y based on whether its an EGL image or not */
      if (wrap->flags & IMAGE_FLAG_DISPLAY)
         y = (wrap->height - 1) - y;

      log2_utile_width = khrn_image_get_log2_utile_width(wrap->format);
      log2_utile_height = khrn_image_get_log2_utile_height(wrap->format);

      ut_w = khrn_image_wrap_get_width_ut(wrap);
      ut_x = x >> log2_utile_width;
      ut_y = y >> log2_utile_height;

      ut_base = (uint8_t *)wrap->storage +
         (khrn_tformat_utile_addr(ut_w, ut_x, ut_y, khrn_image_is_tformat(wrap->format), NULL) * 64);

      x &= (1 << log2_utile_width) - 1;
      y &= (1 << log2_utile_height) - 1;

      switch (wrap->format & IMAGE_FORMAT_PIXEL_SIZE_MASK) {
      case IMAGE_FORMAT_1:  pixel = get_bit((uint8_t *)ut_base + (y * 8) + (x >> 3), x & 0x7); break;
      case IMAGE_FORMAT_4:  pixel = get_nibble((uint8_t *)ut_base + (y * 8) + (x >> 1), x & 0x1); break;
      case IMAGE_FORMAT_8:  pixel = ((uint8_t *)ut_base)[(y * 8) + x]; break;
      case IMAGE_FORMAT_16: pixel = ((uint16_t *)ut_base)[(y * 8) + x]; break;
      case IMAGE_FORMAT_32: pixel = ((uint32_t *)ut_base)[(y * 4) + x]; break;
      default:              UNREACHABLE();
      }

      break;
   }
   default:
   {
      UNREACHABLE();
   }
   }

   return pixel;
}

static uint32_t get_packed_mask_pixel(const KHRN_IMAGE_WRAP_T *wrap, uint32_t x, uint32_t y)
{
   uint32_t channel;
   void *ut_base;

   assert(khrn_image_is_packed_mask(wrap->format) && (khrn_image_is_tformat(wrap->format) || khrn_image_is_lineartile(wrap->format)));
   assert(x < wrap->width);
   assert(y < wrap->height);

   channel = (y / VG_Q_TILE_HEIGHT) & 0x3;
   y = ((y / (VG_Q_TILE_HEIGHT * 4)) * VG_Q_TILE_HEIGHT) + (y & (VG_Q_TILE_HEIGHT - 1));

   ut_base = (uint8_t *)wrap->storage +
      (khrn_tformat_utile_addr(khrn_image_wrap_get_width_ut(wrap), x >> 2, y >> 2, khrn_image_is_tformat(wrap->format), NULL) * 64);
   return ((uint8_t *)ut_base)[((y & 3) * 16) + ((x & 3) * 4) + channel];
}

static INLINE void put_bit(uint8_t *addr, uint32_t bit, uint32_t value)
{
   assert(bit < 8);
   assert(value < 2);

   *addr = (uint8_t)((*addr & ~(1 << bit)) | (value << bit));
}

static INLINE void put_nibble(uint8_t *addr, uint32_t nibble, uint32_t value)
{
   assert(nibble < 2);
   assert(value < 16);

   nibble <<= 2;

   *addr = (uint8_t)((*addr & ~(0xf << nibble)) | (value << nibble));
}

void khrn_image_wrap_put_pixel(KHRN_IMAGE_WRAP_T *wrap, uint32_t x, uint32_t y, uint32_t pixel)
{
   assert(khrn_image_is_uncomp(wrap->format));
   assert(x < wrap->width);
   assert(y < wrap->height);

   switch (wrap->format & IMAGE_FORMAT_MEM_LAYOUT_MASK) {
   case IMAGE_FORMAT_RSO:
   {
      /* either the start or end of the image (depending on whether its marked as IMAGE_FLAG_DISPLAY or not.  Pixmaps and Window surfaces should be */
      void *row = (uint8_t *)wrap->storage + (((wrap->flags & IMAGE_FLAG_DISPLAY) ? 1 : 0) * (wrap->stride * (wrap->height - 1)));
      /* egl images traverse down the image, with first pixel in memory being GL's last row */
      row = (uint8_t *)row + (y * (((wrap->flags & IMAGE_FLAG_DISPLAY) ? -1 : 1) * wrap->stride));

      switch (wrap->format & IMAGE_FORMAT_PIXEL_SIZE_MASK) {
      case IMAGE_FORMAT_1:  put_bit((uint8_t *)row + (x >> 3), x & 0x7, pixel); break;
      case IMAGE_FORMAT_4:  put_nibble((uint8_t *)row + (x >> 1), x & 0x1, pixel); break;
      case IMAGE_FORMAT_8:  ((uint8_t *)row)[x] = (uint8_t)pixel; break;
      case IMAGE_FORMAT_16: assert(!((uintptr_t)row & 1));
         if (khrn_image_is_yuv422(wrap->format))
         {
            if (x & 1)
            {
               ((uint8_t *)row)[2*x+0] = (uint8_t)(pixel>>0 & 0xff);
               ((uint8_t *)row)[2*x+1] = (uint8_t)(pixel>>16 & 0xff);
            }
            else
            {
               ((uint8_t *)row)[2*x+0] = (uint8_t)(pixel>>0 & 0xff);
               ((uint8_t *)row)[2*x+1] = (uint8_t)(pixel>>8 & 0xff);
            }
         }
         else
            ((uint16_t *)row)[x] = (uint16_t)pixel;
         break;
      case IMAGE_FORMAT_32:
         assert(!((uintptr_t)row & 1));
         ((uint32_t *)row)[x] = pixel;
         break;
      case IMAGE_FORMAT_24:
         ((uint8_t *)row)[3*x+2] = (uint8_t)(pixel>>16 & 0xff);
         ((uint8_t *)row)[3*x+1] = (uint8_t)(pixel>>8 & 0xff);
         ((uint8_t *)row)[3*x+0] = (uint8_t)(pixel & 0xff);
         break;
      default:              UNREACHABLE();
      }

      break;
   }
   case IMAGE_FORMAT_TF:
   case IMAGE_FORMAT_LT:
   {
      uint32_t log2_utile_width, log2_utile_height;
      uint32_t ut_w, ut_x, ut_y;
      void *ut_base;

      /* modify y based on whether its an EGL image or not */
      if (wrap->flags & IMAGE_FLAG_DISPLAY)
         y = wrap->height - y;

      log2_utile_width = khrn_image_get_log2_utile_width(wrap->format);
      log2_utile_height = khrn_image_get_log2_utile_height(wrap->format);

      ut_w = khrn_image_wrap_get_width_ut(wrap);
      ut_x = x >> log2_utile_width;
      ut_y = y >> log2_utile_height;

      ut_base = (uint8_t *)wrap->storage +
         (khrn_tformat_utile_addr(ut_w, ut_x, ut_y, khrn_image_is_tformat(wrap->format), NULL) * 64);

      x &= (1 << log2_utile_width) - 1;
      y &= (1 << log2_utile_height) - 1;

      switch (wrap->format & IMAGE_FORMAT_PIXEL_SIZE_MASK) {
      case IMAGE_FORMAT_1:  put_bit((uint8_t *)ut_base + (y * 8) + (x >> 3), x & 0x7, pixel); break;
      case IMAGE_FORMAT_4:  put_nibble((uint8_t *)ut_base + (y * 8) + (x >> 1), x & 0x1, pixel); break;
      case IMAGE_FORMAT_8:  ((uint8_t *)ut_base)[(y * 8) + x] = (uint8_t)pixel; break;
      case IMAGE_FORMAT_16: ((uint16_t *)ut_base)[(y * 8) + x] = (uint16_t)pixel; break;
      case IMAGE_FORMAT_32: ((uint32_t *)ut_base)[(y * 4) + x] = pixel; break;
      default:              UNREACHABLE();
      }

      break;
   }
   default:
   {
      UNREACHABLE();
   }
   }
}

void khrn_image_wrap_put_etc1_block(KHRN_IMAGE_WRAP_T *wrap, uint32_t x, uint32_t y, uint32_t word0, uint32_t word1)
{
   uint32_t l2utw, l2uth, ut_x, ut_y, ut_w;
   void *ut_base;

   assert(khrn_image_is_etc1(wrap->format) && (khrn_image_is_tformat(wrap->format) || khrn_image_is_lineartile(wrap->format)));
   assert(4 * x < wrap->width);
   assert(4 * y < wrap->height);

   l2utw = 1;   //utile is 2 blocks wide
   l2uth = 2;   //utile is 4 blocks high
   ut_x = x >> l2utw;
   ut_y = y >> l2uth;

   ut_w = khrn_image_wrap_get_width_ut(wrap);

   ut_base = (uint8_t *)wrap->storage + khrn_tformat_utile_addr(ut_w, ut_x, ut_y, khrn_image_is_tformat(wrap->format), NULL) * 64;

   x &= (1 << l2utw) - 1;
   y &= (1 << l2uth) - 1;

   assert(x < 2);
   assert(y < 4);

   ((uint32_t *)ut_base)[x * 2 + y * 4]     = word0;
   ((uint32_t *)ut_base)[x * 2 + y * 4 + 1] = word1;
}

uint32_t khrn_image_pixel_to_rgba(KHRN_IMAGE_FORMAT_T format, uint32_t pixel, KHRN_IMAGE_CONV_T conv)
{
   uint32_t rgba = 0;

   assert(khrn_image_is_color(format));

   switch (format & (IMAGE_FORMAT_PIXEL_SIZE_MASK | IMAGE_FORMAT_PIXEL_LAYOUT_MASK)) {
   case (IMAGE_FORMAT_32 | IMAGE_FORMAT_8888):
   {
      assert((format & IMAGE_FORMAT_RGB) && !(format & IMAGE_FORMAT_L));
      rgba = pixel;
      break;
   }
   case (IMAGE_FORMAT_24 | IMAGE_FORMAT_888):
   {
      assert((format & IMAGE_FORMAT_RGB) && !(format & IMAGE_FORMAT_L));
      rgba = pixel | 0xff000000;
      break;
   }
   case (IMAGE_FORMAT_16 | IMAGE_FORMAT_YUYV):
   {
      assert(format & IMAGE_FORMAT_YUV);
      rgba = pixel | 0xff000000;
      break;
   }
   case (IMAGE_FORMAT_16 | IMAGE_FORMAT_4444):
   {
      assert((format & IMAGE_FORMAT_RGB) && !(format & IMAGE_FORMAT_L));
      rgba =
         (((pixel >> 0) & 0xf) * 0x00000011) |
         (((pixel >> 4) & 0xf) * 0x00001100) |
         (((pixel >> 8) & 0xf) * 0x00110000) |
         (((pixel >> 12) & 0xf) * 0x11000000);
      break;
   }
   case (IMAGE_FORMAT_16 | IMAGE_FORMAT_5551):
   {
      assert((format & IMAGE_FORMAT_RGB) && !(format & IMAGE_FORMAT_L));
      if (format & IMAGE_FORMAT_XA) {
         format = (KHRN_IMAGE_FORMAT_T)(format & ~IMAGE_FORMAT_XA);
         pixel = (pixel >> 1) | ((pixel & 0x1) << 15);
      }
      rgba =
         ((((pixel & 0x001f) * 0x21) >> (0 + 2)) << 0) |
         ((((pixel & 0x03e0) * 0x21) >> (5 + 2)) << 8) |
         ((((pixel & 0x7c00) * 0x21) >> (10 + 2)) << 16) |
         ((pixel >> 15) * 0xff000000);
      break;
   }
   case (IMAGE_FORMAT_16 | IMAGE_FORMAT_565):
   {
      assert((format & IMAGE_FORMAT_RGB) && !(format & (IMAGE_FORMAT_L | IMAGE_FORMAT_A | IMAGE_FORMAT_XA)));
      rgba =
         ((((pixel & 0x001f) * 0x21) >> (0 + 2)) << 0) |
         ((((pixel & 0x07e0) * 0x41) >> (5 + 4)) << 8) |
         ((((pixel & 0xf800) * 0x21) >> (11 + 2)) << 16);
      break;
   }
   case (IMAGE_FORMAT_16 | IMAGE_FORMAT_88):
   {
      assert(!(format & (IMAGE_FORMAT_RGB | IMAGE_FORMAT_XRGBX)) && (format & IMAGE_FORMAT_L));
      if (format & IMAGE_FORMAT_XA) {
         rgba = pixel | ((pixel & 0xff00) * 0x00010100);
      } else {
         rgba = ((pixel & 0xff) * 0x00010101) | ((pixel & 0xff00) << 16);
      }
      break;
   }
   case IMAGE_FORMAT_8:
   case IMAGE_FORMAT_4:
   case IMAGE_FORMAT_1:
   {
      assert(!(format & (IMAGE_FORMAT_RGB | IMAGE_FORMAT_XRGBX | IMAGE_FORMAT_XA)) && (!!(format & IMAGE_FORMAT_L) ^ !!(format & IMAGE_FORMAT_A)));
      switch (format & IMAGE_FORMAT_PIXEL_SIZE_MASK) {
      case IMAGE_FORMAT_8: rgba = pixel * 0x01010101; break;
      case IMAGE_FORMAT_4: rgba = pixel * 0x11111111; break;
      case IMAGE_FORMAT_1: rgba = pixel * 0xffffffff; break;
      default:             UNREACHABLE();
      }
      break;
   }
   default:
   {
      UNREACHABLE();
   }
   }

   if (format & IMAGE_FORMAT_XA) {
      rgba = (rgba >> 8) | (rgba << 24);
   }

   if (format & IMAGE_FORMAT_XRGBX) {
      rgba =
         (((rgba >> 0) & 0xff) << 16) |
         (((rgba >> 8) & 0xff) << 8) |
         (((rgba >> 16) & 0xff) << 0) |
         (rgba & 0xff000000);
   }

   if (!(format & IMAGE_FORMAT_A)) {
      rgba |= 0xff000000;
   }

   if (!(format & (IMAGE_FORMAT_RGB | IMAGE_FORMAT_L | IMAGE_FORMAT_YUV))) {
      /*
         in gl-land alpha-only textures are implicitly black
         in vg-land alpha-only textures are implicitly white (i'll assume this is the odd behaviour)
      */

      if (conv == IMAGE_CONV_VG) {
         rgba |= 0x00ffffff;
      } else {
         rgba &= 0xff000000;
      }
   }

   if ((conv == IMAGE_CONV_VG) && khrn_image_is_premultiplied(format)) {
      rgba = khrn_color_rgba_clamp_to_a(rgba);
   }

   return rgba;
}

static INLINE uint32_t to_1(uint32_t x)
{
   return x >> 7;
}

static INLINE uint32_t to_4(uint32_t x)
{
   return ((x * 0xf) + 0x87) >> 8;
}

static INLINE uint32_t to_5(uint32_t x)
{
   if (x >= 128) { ++x; }
   x += 3 - (x >> 5);
   return x >> 3;
}

static INLINE uint32_t to_6(uint32_t x)
{
   if (x <= 84) { ++x; }
   if (x >= 171) { --x; }
   return x >> 2;
}

/*
   luminance values are taken from the red channel
*/

uint32_t khrn_image_rgba_to_pixel(KHRN_IMAGE_FORMAT_T format, uint32_t rgba, KHRN_IMAGE_CONV_T conv)
{
   uint32_t pixel = 0;

   assert(khrn_image_is_color(format));

   if (format & IMAGE_FORMAT_XRGBX) {
      rgba =
         (((rgba >> 0) & 0xff) << 16) |
         (((rgba >> 8) & 0xff) << 8) |
         (((rgba >> 16) & 0xff) << 0) |
         (rgba & 0xff000000);
   }

   if (format & IMAGE_FORMAT_XA &&
      ((format & (IMAGE_FORMAT_PIXEL_SIZE_MASK | IMAGE_FORMAT_PIXEL_LAYOUT_MASK)) != (IMAGE_FORMAT_16 | IMAGE_FORMAT_5551))) {
      rgba = (rgba << 8) | (rgba >> 24);
   }

   switch (format & (IMAGE_FORMAT_PIXEL_SIZE_MASK | IMAGE_FORMAT_PIXEL_LAYOUT_MASK)) {
   case (IMAGE_FORMAT_32 | IMAGE_FORMAT_8888):
   {
      assert((format & IMAGE_FORMAT_RGB) && !(format & IMAGE_FORMAT_L));
      pixel = rgba;
      break;
   }
   case (IMAGE_FORMAT_24 | IMAGE_FORMAT_888):
   {
      assert((format & IMAGE_FORMAT_RGB) && !(format & IMAGE_FORMAT_L));
      pixel = rgba & 0xffffff;
      break;
   }
   case (IMAGE_FORMAT_16 | IMAGE_FORMAT_4444):
   {
      assert((format & IMAGE_FORMAT_RGB) && !(format & IMAGE_FORMAT_L));
      pixel =
         (to_4((rgba >> 0) & 0xff) << 0) |
         (to_4((rgba >> 8) & 0xff) << 4) |
         (to_4((rgba >> 16) & 0xff) << 8) |
         (to_4((rgba >> 24) & 0xff) << 12);
      break;
   }
   case (IMAGE_FORMAT_16 | IMAGE_FORMAT_5551):
   {
      assert((format & IMAGE_FORMAT_RGB) && !(format & IMAGE_FORMAT_L));
      pixel =
         (to_5((rgba >> 0) & 0xff) << 0) |
         (to_5((rgba >> 8) & 0xff) << 5) |
         (to_5((rgba >> 16) & 0xff) << 10) |
         (to_1((rgba >> 24) & 0xff) << 15);
      if ((conv == IMAGE_CONV_VG) && khrn_image_is_premultiplied(format) && !(pixel & (1 << 15))) {
         /* keep rgb <= a */
         pixel = 0;
      }
      if (format & IMAGE_FORMAT_XA) {
         pixel = ((pixel & 0x7fff) << 1) | (pixel >> 15);
      }
      break;
   }
   case (IMAGE_FORMAT_16 | IMAGE_FORMAT_565):
   {
      assert((format & IMAGE_FORMAT_RGB) && !(format & (IMAGE_FORMAT_L | IMAGE_FORMAT_A | IMAGE_FORMAT_XA)));
      pixel =
         (to_5((rgba >> 0) & 0xff) << 0) |
         (to_6((rgba >> 8) & 0xff) << 5) |
         (to_5((rgba >> 16) & 0xff) << 11);
      break;
   }
   case (IMAGE_FORMAT_16 | IMAGE_FORMAT_88):
   {
      assert(!(format & (IMAGE_FORMAT_RGB | IMAGE_FORMAT_XRGBX)) && (format & IMAGE_FORMAT_L));
      if (format & IMAGE_FORMAT_XA) {
         pixel = rgba & 0xffff;
      } else {
         pixel = (rgba & 0xff) | ((rgba >> 16) & 0xff00);
      }
      break;
   }
   case IMAGE_FORMAT_8:
   case IMAGE_FORMAT_4:
   case IMAGE_FORMAT_1:
   {
      assert(!(format & (IMAGE_FORMAT_RGB | IMAGE_FORMAT_XRGBX | IMAGE_FORMAT_XA)) && (!!(format & IMAGE_FORMAT_L) ^ !!(format & IMAGE_FORMAT_A)));
      rgba = (format & IMAGE_FORMAT_L) ? (rgba & 0xff) : (rgba >> 24);
      switch (format & IMAGE_FORMAT_PIXEL_SIZE_MASK) {
      case IMAGE_FORMAT_8: pixel = rgba; break;
      case IMAGE_FORMAT_4: pixel = to_4(rgba); break;
      case IMAGE_FORMAT_1: pixel = to_1(rgba); break;
      default:             UNREACHABLE();
      }
      break;
   }
   default:
   {
      UNREACHABLE();
   }
   }

   return pixel;
}

uint32_t khrn_image_rgba_convert_pre_lin(KHRN_IMAGE_FORMAT_T dst_format, KHRN_IMAGE_FORMAT_T src_format, uint32_t rgba)
{
   if (khrn_image_is_linear(dst_format ^ src_format) ||
      khrn_image_is_premultiplied(dst_format ^ src_format)) {
      if (khrn_image_is_premultiplied(src_format)) {
         rgba = khrn_color_rgba_unpre(rgba);
      }
      if (khrn_image_is_linear(dst_format ^ src_format)) {
         if (khrn_image_is_linear(dst_format)) {
            rgba = khrn_color_rgba_s_to_lin(rgba);
         } else {
            rgba = khrn_color_rgba_lin_to_s(rgba);
         }
      }
      if (khrn_image_is_premultiplied(dst_format)) {
         rgba = khrn_color_rgba_pre(rgba);
      }
   }
   return rgba;
}

uint32_t khrn_image_rgba_convert_l_pre_lin(KHRN_IMAGE_FORMAT_T dst_format, KHRN_IMAGE_FORMAT_T src_format, uint32_t rgba)
{
   if ((dst_format & IMAGE_FORMAT_L) && !(src_format & IMAGE_FORMAT_L)) {
      if (!khrn_image_is_linear(src_format) && khrn_image_is_premultiplied(src_format)) {
         rgba = khrn_color_rgba_unpre(rgba);
         src_format = (KHRN_IMAGE_FORMAT_T)(src_format & ~IMAGE_FORMAT_PRE);
      }
      if (khrn_image_is_linear(dst_format) || khrn_image_is_linear(src_format)) {
         if (!khrn_image_is_linear(src_format)) {
            rgba = khrn_color_rgba_s_to_lin(rgba);
            src_format = khrn_image_to_linear_format(src_format);
         }
         rgba = khrn_color_rgba_to_la_lin(rgba);
      } else {
         rgba = khrn_color_rgba_to_la_s(rgba);
      }
   }
   return khrn_image_rgba_convert_pre_lin(dst_format, src_format, rgba);
}

void khrn_image_wrap_clear_region(
   KHRN_IMAGE_WRAP_T *wrap, uint32_t x, uint32_t y,
   uint32_t width, uint32_t height,
   uint32_t rgba, /* rgba non-lin, unpre */
   KHRN_IMAGE_CONV_T conv)
{
   uint32_t pixel, i, j;

   assert(khrn_image_is_color(wrap->format));
   assert((x + width) <= wrap->width);
   assert((y + height) <= wrap->height);

   if (khrn_bf_clear_region(wrap, x, y, width, height, rgba, conv))
      return;

   if (conv == IMAGE_CONV_VG) {
      rgba = khrn_image_rgba_convert_l_pre_lin(wrap->format, (KHRN_IMAGE_FORMAT_T)0 /* rgba, non-lin, non-pre */, rgba);
   }
   pixel = khrn_image_rgba_to_pixel(wrap->format, rgba, conv);

   for (j = 0; j != height; ++j) {
      for (i = 0; i != width; ++i) {
         khrn_image_wrap_put_pixel(wrap, x + i, y + j, pixel);
      }
   }
}

static void copy_region_from_packed_mask_tf(
   KHRN_IMAGE_WRAP_T *dst, uint32_t dst_x, uint32_t dst_y,
   uint32_t width, uint32_t height,
   const KHRN_IMAGE_WRAP_T *src, uint32_t src_x, uint32_t src_y)
{
   uint32_t i, j;

   assert((khrn_image_is_uncomp(dst->format) && (khrn_image_get_bpp(dst->format) == 8)) && (src->format == PACKED_MASK_TF));
   assert((dst_x + width) <= dst->width);
   assert((dst_y + height) <= dst->height);
   assert((src_x + width) <= src->width);
   assert((src_y + height) <= src->height);

   for (j = 0; j != height; ++j) {
      for (i = 0; i != width; ++i) {
         khrn_image_wrap_put_pixel(dst, dst_x + i, dst_y + j, get_packed_mask_pixel(src, src_x + i, src_y + j));
      }
   }
}

static void copy_pixel(
   KHRN_IMAGE_WRAP_T *dst, uint32_t dst_x, uint32_t dst_y,
   const KHRN_IMAGE_WRAP_T *src, uint32_t src_x, uint32_t src_y,
   KHRN_IMAGE_CONV_T conv)
{
   uint32_t pixel;

   assert(khrn_image_is_uncomp(dst->format) && khrn_image_is_uncomp(src->format));
   assert(dst_x < dst->width);
   assert(dst_y < dst->height);
   assert(src_x < src->width);
   assert(src_y < src->height);

   pixel = khrn_image_wrap_get_pixel(src, src_x, src_y);

   if (((dst->format ^ src->format) & ~IMAGE_FORMAT_MEM_LAYOUT_MASK) ||
      /* need khrn_image_pixel_to_rgba to clamp rgb to a */
      ((conv == IMAGE_CONV_VG) &&
       khrn_image_is_color(src->format) &&
       khrn_image_is_premultiplied(src->format))
       ) {
      if (khrn_image_is_paletted(src->format) && khrn_image_is_color(dst->format)) {
         /* we can convert from paletted to non-paletted by doing palette lookups */
         uint32_t rgba;

         assert(src->palette);
         assert(pixel < 16);

         rgba = ((uint32_t *)src->palette)[pixel];
         /* TODO: would lum/pre/lin conversion make sense here? */
         pixel = khrn_image_rgba_to_pixel(dst->format, rgba, conv);
      } else {
         uint32_t rgba;

         assert(khrn_image_is_color(dst->format) && khrn_image_is_color(src->format));

         rgba = khrn_image_pixel_to_rgba(src->format, pixel, conv);
         if (conv == IMAGE_CONV_VG) {
            rgba = khrn_image_rgba_convert_l_pre_lin(dst->format, src->format, rgba);
         }
         pixel = khrn_image_rgba_to_pixel(dst->format, rgba, conv);
      }
   }

   khrn_image_wrap_put_pixel(dst, dst_x, dst_y, pixel);
}

static bool in_scissor_rect(
   uint32_t x, uint32_t y,
   const int32_t *scissor_rects, uint32_t scissor_rects_count)
{
   uint32_t i;
   for (i = 0; i != scissor_rects_count; i += 4) {
      int32_t rect_x      = scissor_rects[i];
      int32_t rect_y      = scissor_rects[i + 1];
      int32_t rect_width  = scissor_rects[i + 2];
      int32_t rect_height = scissor_rects[i + 3];
      if (((int32_t)x >= rect_x) && ((int32_t)x < _adds(rect_x, rect_width)) &&
         ((int32_t)y >= rect_y) && ((int32_t)y < _adds(rect_y, rect_height))) {
         return true;
      }
   }
   return false;
}

typedef struct
{
   KHRN_IMAGE_WRAP_T       *dst;
   uint32_t                dst_x;
   uint32_t                dst_y;
   uint32_t                width;
   uint32_t                height;
   const KHRN_IMAGE_WRAP_T *src;
   uint32_t                src_x;
   uint32_t                src_y;
   KHRN_IMAGE_CONV_T       conv;
   const int32_t           *scissor_rects;
   uint32_t                scissor_rects_count;
   bool                    retCode;
} KHRN_COPY_SCISSOR_REGIONS_ARGS_T;

static void copy_scissor_regions(
   KHRN_IMAGE_WRAP_T *dst, uint32_t dst_x, uint32_t dst_y,
   uint32_t width, uint32_t height,
   const KHRN_IMAGE_WRAP_T *src, uint32_t src_x, uint32_t src_y,
   KHRN_IMAGE_CONV_T conv,
   const int32_t *scissor_rects, uint32_t scissor_rects_count)
{
   int32_t begin_j, end_j, dir_j;
   int32_t begin_i, end_i, dir_i;
   int32_t i, j;

   assert(khrn_image_is_uncomp(dst->format) && khrn_image_is_uncomp(src->format));
   assert((dst_x + width) <= dst->width);
   assert((dst_y + height) <= dst->height);
   assert((src_x + width) <= src->width);
   assert((src_y + height) <= src->height);

   /*
      choose blitting direction to handle overlapping src/dst
   */

   if (dst_y < src_y) {
      begin_j = 0;
      end_j = height;
      dir_j = 1;
   } else {
      begin_j = height - 1;
      end_j = -1;
      dir_j = -1;
   }

   if (dst_x < src_x) {
      begin_i = 0;
      end_i = width;
      dir_i = 1;
   } else {
      begin_i = width - 1;
      end_i = -1;
      dir_i = -1;
   }

   for (j = begin_j; j != end_j; j += dir_j) {
      for (i = begin_i; i != end_i; i += dir_i) {
         if (!scissor_rects || in_scissor_rect(dst_x + i, dst_y + j, scissor_rects, scissor_rects_count)) {
            copy_pixel(
               dst, dst_x + i, dst_y + j,
               src, src_x + i, src_y + j,
               conv);
         }
      }
   }
}

static bool part_copy_scissor_regions(ABSTRACT_SERVER_ARGS_T *args)
{
   KHRN_COPY_SCISSOR_REGIONS_ARGS_T *copyArgs = (KHRN_COPY_SCISSOR_REGIONS_ARGS_T*)args->args;
   uint32_t part     = args->part;
   uint32_t numParts = args->numParts;

   uint32_t sy, dy, height;
   uint32_t partHeight = copyArgs->height / numParts;

   sy = copyArgs->src_y + (part * partHeight);
   dy = copyArgs->dst_y + (part * partHeight);

   height = partHeight;
   if (part == numParts - 1)
      height = copyArgs->height - (partHeight * part);

   if (height == 0)
      return true;

   copy_scissor_regions(copyArgs->dst, copyArgs->dst_x, dy, copyArgs->width, height,
                        copyArgs->src, copyArgs->src_x, sy, copyArgs->conv,
                        copyArgs->scissor_rects, copyArgs->scissor_rects_count);

   return true;
}

static void par_copy_scissor_regions(
   KHRN_IMAGE_WRAP_T *dst, uint32_t dst_x, uint32_t dst_y,
   uint32_t width, uint32_t height,
   const KHRN_IMAGE_WRAP_T *src, uint32_t src_x, uint32_t src_y,
   KHRN_IMAGE_CONV_T conv,
   const int32_t *scissor_rects, uint32_t scissor_rects_count)
{
   uint32_t numWorkers = khrn_par_num_workers();
   bool parallel = true;

   /* the src and destination overlap, then its not appropriate to use a parallel copy */
   if (((uint32_t)src->storage >= (uint32_t)dst->storage) &&
       ((uint32_t)src->storage <= ((uint32_t)dst->storage + khrn_image_get_size(dst->format, width, height))))
   {
      parallel = false;
   }

   if (parallel && numWorkers > 1 && width * height >= 1024 && height >= numWorkers)   /* TODO : tune size */
   {
      KHRN_COPY_SCISSOR_REGIONS_ARGS_T   copyArgs;
      copyArgs.conv     = conv;
      copyArgs.dst      = dst;
      copyArgs.dst_x    = dst_x;
      copyArgs.dst_y    = dst_y;
      copyArgs.src      = src;
      copyArgs.src_x    = src_x;
      copyArgs.src_y    = src_y;
      copyArgs.width    = width;
      copyArgs.height   = height;
      copyArgs.scissor_rects = scissor_rects;
      copyArgs.scissor_rects_count = scissor_rects_count;

      khrn_par_server_job(part_copy_scissor_regions, &copyArgs);
   }
   else
      copy_scissor_regions(dst, dst_x, dst_y, width, height, src, src_x, src_y, conv, scissor_rects, scissor_rects_count);
}

/* TFORMAT ACCELERATION ROUTINES
 *
 * Copying from linear to TFORMAT follows the following algorithm:
 *
 * For each 4k TILE
 *    for each tile row
 *       for each pixel in row
 *           copy from linear to Tformat
 *
 * Thus, the linear memory is accessed sequentially for each row and the data scattered
 * into the TFORMAT block.  Most of the address calculation is performed in the outer loops
 * with simple offsets calculated in the inner loops.
 *
 * Each routine has two paths -- a fast path that copies (groups of) texels as integers,
 * and a slow path that copies individual texels.  The slow path is used at the edges of
 * an image if it does not coincide with a tile boundary, or when the aligment of the data
 * does not allow an integer copy.
 *
 * Addresses in the TFORMAT image are calculated using lookup tables/bitwise operations
 *
 * For a texel addressed by (X, Y):
 *
 * TFORMAT (for 32bpp) address calculated as:
 *
 * +---------------------------------------------------------------------------------+
 * | TILE ADDRESSS | MAP[Y5:Y4:X4] | Y3 | Y2 | X3 | X2 | Y1 | Y0 | X1 | X0 |  0 |  0 |
 * +---------------------------------------------------------------------------------+
 *
 * TFORMAT (for 16bpp) address calculated as:
 *
 * +---------------------------------------------------------------------------------+
 * | TILE ADDRESSS | MAP[Y5:Y4:X5] | Y3 | Y2 | X4 | X3 | Y1 | Y0 | X2 | X1 | X0 |  0 |
 * +---------------------------------------------------------------------------------+
 *
 * TFORMAT (for 8bpp) address calculated as:
 *
 * +---------------------------------------------------------------------------------+
 * | TILE ADDRESSS | MAP[Y6:Y5:X5] | Y4 | Y3 | X4 | X3 | Y2 | Y1 | Y0 | X2 | X1 | X0 |
 * +---------------------------------------------------------------------------------+
 *
 * The low order bits of the address are calculated from the subX and subY position
 * using the appropriate XSWIZZLE/YSWIZZLE macros
 *
 * MAP converts its argument bits into the tile address offset.  The Y5 (32 and 16bpp) or
 *     Y6 bit selects between even and odd tile rows (which have inverted horizontal
 *     tile order and different sub-tile addressing).
 *           0x000, 0xc00, 0x400, 0x800,
 *           0x800, 0x400, 0xc00, 0x000
 *
 * MAP is encoded in a single word in the routines below to minimise memory accesses
 * for array lookups.
 *
 * TILE ADDRESS is calculated from the block X and Y position, noting that the X position
 *              is inverted if Y5 (Y6 for 8bit) = 1 (odd block rows)
 */


/* NOTE on optimisations:
 *
 * The routines below have three modes of operation:
 *  1) extra fast: a whole row of microtiles within a tile is copied writing to linear addresses within the tile
 *                 which attempts to maximise cache coherence - this should benefit most architectures
 *  2) fast      : a whole scan-line is read linearly and written to the scattered tile addresses
 *                 which is used when there isn't a whole micro-tile height available, but there is a full tile width
 *  3) slow      : single texels are processed
 *                 which is used when processing partial tiles (typically at the edges)
 *
 * The MIPS processors support a prefetch instruction.  We use two modes for optimising performance
 *
 *   mode 4:  streamed data prefetch (see macro "fetch" below)
 *   mode 30: prepare for store which avoids loading a cache line if we guarantee to write it all (macro "store" below)
 *
 * In extra fast mode, the prepare for store MIPs optimisation is used.
 *
 * Prepare for store assumes that the whole cache line is written.  The optimisations below should work
 * provided that the cache line size does not exceed 256. It is e.g. 32 on 7425.
 */

const uint32_t  MAP_ODD  = 0x0c48;
const uint32_t  MAP_EVEN = 0x84c0;

/* These convert the lower four/five bits of x-address into the corresponding bits of the Tformat address
 */
#define XSWIZZLE32(X)  ((((X) & 0x3) << 0) | (((X) & 0xc ) << 2)) // 32-bit offset
#define XSWIZZLE16(X)  ((((X) & 0x7) << 0) | (((X) & 0x18) << 2)) // 16 bit offset
#define XSWIZZLE8(X)   ((((X) & 0x7) << 0) | (((X) & 0x18) << 3)) //  8 bit offset

/* These converts the lower four/five bits of y-address into the corresponding bits of the Tformat address
 */
#define YSWIZZLE32(Y)  ((((Y) & 0x3) << 4) | (((Y) & 0xc ) << 6)) // 8-bit offset
#define YSWIZZLE16(Y)  ((((Y) & 0x3) << 4) | (((Y) & 0xc ) << 6)) // 8-bit offset
#define YSWIZZLE8(Y)   ((((Y) & 0x7) << 3) | (((Y) & 0x18) << 5)) // 8-bit offset

/* These decode YUYV words into UV and Y1,Y2 components */

/* For 8888 */
#ifdef BIG_ENDIAN_CPU

#define UV32(a) (((a & 0xFF0000)) | ((a & 0xFF) << 8))
#define Y132(a) ((a & 0xFF000000))
#define Y232(a) ((a & 0xFF00) << 16)

#define UV32_REV(a) (((a & 0xFF000000) >> 8) | (a & 0xFF00))
#define Y132_REV(a) ((a & 0xFF0000) << 8)
#define Y232_REV(a) ((a & 0xFF) << 24)

#else

#define UV32(a) ((a & 0xFF00) | ((a & 0xFF000000) >> 8))
#define Y132(a) (a & 0xFF)
#define Y232(a) ((a & 0xFF0000) >> 16)

#define UV32_REV(a) (((a & 0xFF) << 8) | (a & 0xFF0000))
#define Y132_REV(a) ((a & 0xFF00) >> 8)
#define Y232_REV(a) ((a & 0xFF000000) >> 24)

#endif

#ifdef __mips__
#define fetch(base) __asm__ __volatile__( \
   ".set push      \n"                   \
   ".set noreorder \n"                   \
   "pref 4,(0)(%0);    \n"               \
   ".set pop       \n"                   \
   :                                     \
   : "r" (base))
#endif

#ifdef __arm__
#define fetch __builtin_prefetch
#endif

#ifndef fetch
#define fetch(base) (void)base
#endif

#ifdef __mips__
#define store(base) __asm__ __volatile__( \
    ".set push      \n"                   \
    ".set noreorder \n"                   \
    "pref 30,(0)(%0);    \n"              \
    ".set pop       \n"                   \
    :                                     \
    : "r" (base))
#endif

#ifdef __arm__
#define store __builtin_prefetch
#endif

#ifndef store
#define store(base)
#endif

#ifdef __GNUC__
#define OPTIMIZE_FN __attribute__((optimize("-O3")))
#else
#define OPTIMIZE_FN
#endif

static const uint32_t   TILE_SIZE = 4096;

/* khrn_copy_8888_to_tf32
 *
 * Copies an array in RGBA 32-bit linear format to the equivalent 32-bit Tformat array
 */
static bool OPTIMIZE_FN khrn_copy_8888_to_tf32(
   uint8_t   *dst,                             /* address of Tformat image                                             */
   uint32_t  dstWidth,                         /* width of destination in texels                                       */
   uint32_t  dstStartX, uint32_t dstStartY,    /* bottom left of destination region                                    */
   uint32_t  width,     uint32_t height,       /* top right of destination region                                      */
   uint8_t   *src,                             /* address of linear image                                              */
   uint32_t  srcStartX, uint32_t srcStartY,    /* bottom left of source region                                         */
   uint32_t  srcStride,                        /* stride of source image in bytes (i.e. line length including padding) */
   int32_t   srcDir)                           /* direction in memory                                                  */
{
   static const uint32_t   TILE_XSHIFT       = 5;
   static const uint32_t   TILE_XBITS        = 0x1f;
   static const uint32_t   TILE_YSHIFT       = 5;
   static const uint32_t   TILE_YBITS        = 0x1f;
   static const uint32_t   SUB_TILE_WIDTH    = 16;
   static const uint32_t   TEXEL_SIZE        = 4;
   static const uint32_t   SRC_TILE_STRIDE32 = 32;
   static const uint32_t   SRC_STRIDE32      = 16;

   int32_t    srcStride32 = srcDir * srcStride / 4;
   uint32_t   blockY, blockX, subY, subX;

   uint32_t   dstBlockWidth  = ((dstWidth + TILE_XBITS) & ~TILE_XBITS) >> TILE_XSHIFT;  /* Width in 4k tiles */
   uint32_t   dstBlockStride = dstBlockWidth * TILE_SIZE;

   uint32_t   dstEndX    = dstStartX + width  - 1;
   uint32_t   dstEndY    = dstStartY + height - 1;

   uint32_t   startBlockX = dstStartX >> TILE_XSHIFT;
   uint32_t   startBlockY = dstStartY >> TILE_YSHIFT;
   uint32_t   endBlockX   = dstEndX   >> TILE_XSHIFT;
   uint32_t   endBlockY   = dstEndY   >> TILE_YSHIFT;

   int   offX = srcStartX - dstStartX;
   int   offY = srcStartY - dstStartY;

   /* For each 4k tile vertically */
   for (blockY = startBlockY; blockY <= endBlockY; ++blockY)
   {
      uint32_t   startSubY = blockY == startBlockY ? (dstStartY & TILE_YBITS) : 0;
      uint32_t   endSubY   = blockY == endBlockY   ? (dstEndY   & TILE_YBITS) : TILE_YBITS;

      uint32_t   odd = blockY & 1;
      uint32_t   mask = odd ? MAP_ODD : MAP_EVEN;

      /* For each 4k tile horizontally */
      for (blockX = startBlockX; blockX <= endBlockX; ++blockX)
      {
         uint32_t   startSubX = blockX == startBlockX ? (dstStartX & TILE_XBITS) : 0;
         uint32_t   endSubX   = blockX == endBlockX   ? (dstEndX   & TILE_XBITS) : TILE_XBITS;
         bool       fast      = startSubX == 0 && endSubX == TILE_XBITS;

         uint32_t   blockXR   = odd ? dstBlockWidth - blockX - 1 : blockX;

         /* Address of tile */
         uint8_t     *tileAddr  = dst + blockY * dstBlockStride + blockXR * TILE_SIZE;
         uint32_t    Y = (blockY << TILE_YSHIFT) + offY;
         uint32_t    X = (blockX << TILE_XSHIFT) + startSubX + offX;

         /* For each row in the tile */
         for (subY = startSubY; subY <= endSubY; ++subY)
         {
            uint32_t  *src_texel = (uint32_t *)(src + (Y + subY) * srcDir * srcStride + X * TEXEL_SIZE);

            uint32_t  offset     = mask >> ((subY & 0x10) >> 1);
            uint8_t   *rowAddr   = tileAddr + YSWIZZLE32(subY);
            uint32_t  *rowAddr0  = (uint32_t *)(rowAddr + ((offset & 0x0f) << 8)); /* Start of left sub-tile  */
            uint32_t  *rowAddr1  = (uint32_t *)(rowAddr + ((offset & 0xf0) << 4)); /* Start of right sub-tile */
            bool      extra_fast = fast && ((subY & 0x3) == 0) && ((subY + 0x3) <= endSubY);

            if (extra_fast)
            {
               /* This case copies four lines at a time, and writes a whole destination cache line at a time
                  which allows the use of "prepare for store" prefetch instructions eliminating the need to
                  load data into the cache for immediate overwrite */
               uint32_t *src0 = src_texel;
               uint32_t *src1 = src0 + srcStride32;
               uint32_t *src2 = src1 + srcStride32;
               uint32_t *src3 = src2 + srcStride32;
               uint32_t *dest = rowAddr0;
               uint32_t subtile;

               fetch(&src0[SRC_TILE_STRIDE32]); fetch(&src1[SRC_TILE_STRIDE32]);
               fetch(&src2[SRC_TILE_STRIDE32]); fetch(&src3[SRC_TILE_STRIDE32]);

               for (subtile = 0; subtile < 2; ++subtile)
               {
#ifdef __ARM_NEON__
                  uint32x4_t tmp0, tmp1, tmp2, tmp3;
                  tmp0 = vdupq_n_u32(0);
                  tmp1 = vdupq_n_u32(0);
                  tmp2 = vdupq_n_u32(0);
                  tmp3 = vdupq_n_u32(0);

                  /* missaligned, so separate loads */
                  tmp0 = vld1q_lane_u32(&src0[0], tmp0, 0);
                  tmp0 = vld1q_lane_u32(&src0[1], tmp0, 1);
                  tmp0 = vld1q_lane_u32(&src0[2], tmp0, 2);
                  tmp0 = vld1q_lane_u32(&src0[3], tmp0, 3);

                  tmp1 = vld1q_lane_u32(&src1[0], tmp1, 0);
                  tmp1 = vld1q_lane_u32(&src1[1], tmp1, 1);
                  tmp1 = vld1q_lane_u32(&src1[2], tmp1, 2);
                  tmp1 = vld1q_lane_u32(&src1[3], tmp1, 3);

                  tmp2 = vld1q_lane_u32(&src2[0], tmp2, 0);
                  tmp2 = vld1q_lane_u32(&src2[1], tmp2, 1);
                  tmp2 = vld1q_lane_u32(&src2[2], tmp2, 2);
                  tmp2 = vld1q_lane_u32(&src2[3], tmp2, 3);

                  tmp3 = vld1q_lane_u32(&src3[0], tmp3, 0);
                  tmp3 = vld1q_lane_u32(&src3[1], tmp3, 1);
                  tmp3 = vld1q_lane_u32(&src3[2], tmp3, 2);
                  tmp3 = vld1q_lane_u32(&src3[3], tmp3, 3);

                  /* aligned, 128bits at a time */
                  vst1q_u32(&dest[0x00], tmp0);
                  vst1q_u32(&dest[0x04], tmp1);
                  vst1q_u32(&dest[0x08], tmp2);
                  vst1q_u32(&dest[0x0c], tmp3);

                  /* missaligned, so separate loads */
                  tmp0 = vld1q_lane_u32(&src0[4], tmp0, 0);
                  tmp0 = vld1q_lane_u32(&src0[5], tmp0, 1);
                  tmp0 = vld1q_lane_u32(&src0[6], tmp0, 2);
                  tmp0 = vld1q_lane_u32(&src0[7], tmp0, 3);

                  tmp1 = vld1q_lane_u32(&src1[4], tmp1, 0);
                  tmp1 = vld1q_lane_u32(&src1[5], tmp1, 1);
                  tmp1 = vld1q_lane_u32(&src1[6], tmp1, 2);
                  tmp1 = vld1q_lane_u32(&src1[7], tmp1, 3);

                  tmp2 = vld1q_lane_u32(&src2[4], tmp2, 0);
                  tmp2 = vld1q_lane_u32(&src2[5], tmp2, 1);
                  tmp2 = vld1q_lane_u32(&src2[6], tmp2, 2);
                  tmp2 = vld1q_lane_u32(&src2[7], tmp2, 3);

                  tmp3 = vld1q_lane_u32(&src3[4], tmp3, 0);
                  tmp3 = vld1q_lane_u32(&src3[5], tmp3, 1);
                  tmp3 = vld1q_lane_u32(&src3[6], tmp3, 2);
                  tmp3 = vld1q_lane_u32(&src3[7], tmp3, 3);

                  /* aligned, 128bits at a time */
                  vst1q_u32(&dest[0x10], tmp0);
                  vst1q_u32(&dest[0x14], tmp1);
                  vst1q_u32(&dest[0x18], tmp2);
                  vst1q_u32(&dest[0x1c], tmp3);

                  /* missaligned, so separate loads */
                  tmp0 = vld1q_lane_u32(&src0[8],  tmp0, 0);
                  tmp0 = vld1q_lane_u32(&src0[9],  tmp0, 1);
                  tmp0 = vld1q_lane_u32(&src0[10], tmp0, 2);
                  tmp0 = vld1q_lane_u32(&src0[11], tmp0, 3);

                  tmp1 = vld1q_lane_u32(&src1[8],  tmp1, 0);
                  tmp1 = vld1q_lane_u32(&src1[9],  tmp1, 1);
                  tmp1 = vld1q_lane_u32(&src1[10], tmp1, 2);
                  tmp1 = vld1q_lane_u32(&src1[11], tmp1, 3);

                  tmp2 = vld1q_lane_u32(&src2[8],  tmp2, 0);
                  tmp2 = vld1q_lane_u32(&src2[9],  tmp2, 1);
                  tmp2 = vld1q_lane_u32(&src2[10], tmp2, 2);
                  tmp2 = vld1q_lane_u32(&src2[11], tmp2, 3);

                  tmp3 = vld1q_lane_u32(&src3[8],  tmp3, 0);
                  tmp3 = vld1q_lane_u32(&src3[9],  tmp3, 1);
                  tmp3 = vld1q_lane_u32(&src3[10], tmp3, 2);
                  tmp3 = vld1q_lane_u32(&src3[11], tmp3, 3);

                  /* aligned, 128bits at a time */
                  vst1q_u32(&dest[0x20], tmp0);
                  vst1q_u32(&dest[0x24], tmp1);
                  vst1q_u32(&dest[0x28], tmp2);
                  vst1q_u32(&dest[0x2c], tmp3);

                  /* missaligned, so separate loads */
                  tmp0 = vld1q_lane_u32(&src0[12], tmp0, 0);
                  tmp0 = vld1q_lane_u32(&src0[13], tmp0, 1);
                  tmp0 = vld1q_lane_u32(&src0[14], tmp0, 2);
                  tmp0 = vld1q_lane_u32(&src0[15], tmp0, 3);

                  tmp1 = vld1q_lane_u32(&src1[12], tmp1, 0);
                  tmp1 = vld1q_lane_u32(&src1[13], tmp1, 1);
                  tmp1 = vld1q_lane_u32(&src1[14], tmp1, 2);
                  tmp1 = vld1q_lane_u32(&src1[15], tmp1, 3);

                  tmp2 = vld1q_lane_u32(&src2[12], tmp2, 0);
                  tmp2 = vld1q_lane_u32(&src2[13], tmp2, 1);
                  tmp2 = vld1q_lane_u32(&src2[14], tmp2, 2);
                  tmp2 = vld1q_lane_u32(&src2[15], tmp2, 3);

                  tmp3 = vld1q_lane_u32(&src3[12], tmp3, 0);
                  tmp3 = vld1q_lane_u32(&src3[13], tmp3, 1);
                  tmp3 = vld1q_lane_u32(&src3[14], tmp3, 2);
                  tmp3 = vld1q_lane_u32(&src3[15], tmp3, 3);

                  /* aligned, 128bits at a time */
                  vst1q_u32(&dest[0x30], tmp0);
                  vst1q_u32(&dest[0x34], tmp1);
                  vst1q_u32(&dest[0x38], tmp2);
                  vst1q_u32(&dest[0x3c], tmp3);
#else
                  /* Mark destination cache lines as being fully written so not read */
                  store(&dest[0x00]); store(&dest[0x08]); store(&dest[0x10]); store(&dest[0x18]);
                  dest[0x00] = src0[0];   dest[0x01] = src0[1];   dest[0x02] = src0[2];   dest[0x03] = src0[3];
                  dest[0x04] = src1[0];   dest[0x05] = src1[1];   dest[0x06] = src1[2];   dest[0x07] = src1[3];
                  dest[0x08] = src2[0];   dest[0x09] = src2[1];   dest[0x0a] = src2[2];   dest[0x0b] = src2[3];
                  dest[0x0c] = src3[0];   dest[0x0d] = src3[1];   dest[0x0e] = src3[2];   dest[0x0f] = src3[3];
                  dest[0x10] = src0[4];   dest[0x11] = src0[5];   dest[0x12] = src0[6];   dest[0x13] = src0[7];
                  dest[0x14] = src1[4];   dest[0x15] = src1[5];   dest[0x16] = src1[6];   dest[0x17] = src1[7];
                  dest[0x18] = src2[4];   dest[0x19] = src2[5];   dest[0x1a] = src2[6];   dest[0x1b] = src2[7];
                  dest[0x1c] = src3[4];   dest[0x1d] = src3[5];   dest[0x1e] = src3[6];   dest[0x1f] = src3[7];

                  store(&dest[0x20]); store(&dest[0x28]); store(&dest[0x30]); store(&dest[0x38]);
                  dest[0x20] = src0[8];   dest[0x21] = src0[9];   dest[0x22] = src0[10];  dest[0x23] = src0[11];
                  dest[0x24] = src1[8];   dest[0x25] = src1[9];   dest[0x26] = src1[10];  dest[0x27] = src1[11];
                  dest[0x28] = src2[8];   dest[0x29] = src2[9];   dest[0x2a] = src2[10];  dest[0x2b] = src2[11];
                  dest[0x2c] = src3[8];   dest[0x2d] = src3[9];   dest[0x2e] = src3[10];  dest[0x2f] = src3[11];
                  dest[0x30] = src0[12];  dest[0x31] = src0[13];  dest[0x32] = src0[14];  dest[0x33] = src0[15];
                  dest[0x34] = src1[12];  dest[0x35] = src1[13];  dest[0x36] = src1[14];  dest[0x37] = src1[15];
                  dest[0x38] = src2[12];  dest[0x39] = src2[13];  dest[0x3a] = src2[14];  dest[0x3b] = src2[15];
                  dest[0x3c] = src3[12];  dest[0x3d] = src3[13];  dest[0x3e] = src3[14];  dest[0x3f] = src3[15];
#endif

                  src0 += SRC_STRIDE32; src1 += SRC_STRIDE32;
                  src2 += SRC_STRIDE32; src3 += SRC_STRIDE32;

                  dest = rowAddr1;
               }

               subY += 3; /* Doing 4-lines at a time, so skip another 3 */
            }
            else if (fast)
            {
#ifdef __ARM_NEON__
               uint32x4_t tmp0, tmp1, tmp2, tmp3;
               tmp0 = vdupq_n_u32(0);
               tmp1 = vdupq_n_u32(0);
               tmp2 = vdupq_n_u32(0);
               tmp3 = vdupq_n_u32(0);

               /* Prefetch next tile's source data */
               fetch(&src_texel[SRC_TILE_STRIDE32]);

               /* missaligned, so separate loads */
               tmp0 = vld1q_lane_u32(&src_texel[ 0], tmp0, 0);
               tmp0 = vld1q_lane_u32(&src_texel[ 1], tmp0, 1);
               tmp0 = vld1q_lane_u32(&src_texel[ 2], tmp0, 2);
               tmp0 = vld1q_lane_u32(&src_texel[ 3], tmp0, 3);

               tmp1 = vld1q_lane_u32(&src_texel[ 4], tmp1, 0);
               tmp1 = vld1q_lane_u32(&src_texel[ 5], tmp1, 1);
               tmp1 = vld1q_lane_u32(&src_texel[ 6], tmp1, 2);
               tmp1 = vld1q_lane_u32(&src_texel[ 7], tmp1, 3);

               tmp2 = vld1q_lane_u32(&src_texel[ 8], tmp2, 0);
               tmp2 = vld1q_lane_u32(&src_texel[ 9], tmp2, 1);
               tmp2 = vld1q_lane_u32(&src_texel[10], tmp2, 2);
               tmp2 = vld1q_lane_u32(&src_texel[11], tmp2, 3);

               tmp3 = vld1q_lane_u32(&src_texel[12], tmp3, 0);
               tmp3 = vld1q_lane_u32(&src_texel[13], tmp3, 1);
               tmp3 = vld1q_lane_u32(&src_texel[14], tmp3, 2);
               tmp3 = vld1q_lane_u32(&src_texel[15], tmp3, 3);

               /* aligned, 128bits at a time */
               vst1q_u32(&rowAddr0[0x00], tmp0);
               vst1q_u32(&rowAddr0[0x10], tmp1);
               vst1q_u32(&rowAddr0[0x20], tmp2);
               vst1q_u32(&rowAddr0[0x30], tmp3);

               /* missaligned, so separate loads */
               tmp0 = vld1q_lane_u32(&src_texel[16], tmp0, 0);
               tmp0 = vld1q_lane_u32(&src_texel[17], tmp0, 1);
               tmp0 = vld1q_lane_u32(&src_texel[18], tmp0, 2);
               tmp0 = vld1q_lane_u32(&src_texel[19], tmp0, 3);

               tmp1 = vld1q_lane_u32(&src_texel[20], tmp1, 0);
               tmp1 = vld1q_lane_u32(&src_texel[21], tmp1, 1);
               tmp1 = vld1q_lane_u32(&src_texel[22], tmp1, 2);
               tmp1 = vld1q_lane_u32(&src_texel[23], tmp1, 3);

               tmp2 = vld1q_lane_u32(&src_texel[24], tmp2, 0);
               tmp2 = vld1q_lane_u32(&src_texel[25], tmp2, 1);
               tmp2 = vld1q_lane_u32(&src_texel[26], tmp2, 2);
               tmp2 = vld1q_lane_u32(&src_texel[27], tmp2, 3);

               tmp3 = vld1q_lane_u32(&src_texel[28], tmp3, 0);
               tmp3 = vld1q_lane_u32(&src_texel[29], tmp3, 1);
               tmp3 = vld1q_lane_u32(&src_texel[30], tmp3, 2);
               tmp3 = vld1q_lane_u32(&src_texel[31], tmp3, 3);

               /* aligned, 128bits at a time */
               vst1q_u32(&rowAddr1[0x00], tmp0);
               vst1q_u32(&rowAddr1[0x10], tmp1);
               vst1q_u32(&rowAddr1[0x20], tmp2);
               vst1q_u32(&rowAddr1[0x30], tmp3);
#else
               /* Prefetch next tile's source data */
               fetch(&src_texel[SRC_TILE_STRIDE32]);

               /* Linear read, scatter write for copies that can't easily be batched */
               rowAddr0[0x00] = src_texel[0];   rowAddr0[0x01] = src_texel[1];
               rowAddr0[0x02] = src_texel[2];   rowAddr0[0x03] = src_texel[3];
               rowAddr0[0x10] = src_texel[4];   rowAddr0[0x11] = src_texel[5];
               rowAddr0[0x12] = src_texel[6];   rowAddr0[0x13] = src_texel[7];
               rowAddr0[0x20] = src_texel[8];   rowAddr0[0x21] = src_texel[9];
               rowAddr0[0x22] = src_texel[10];  rowAddr0[0x23] = src_texel[11];
               rowAddr0[0x30] = src_texel[12];  rowAddr0[0x31] = src_texel[13];
               rowAddr0[0x32] = src_texel[14];  rowAddr0[0x33] = src_texel[15];

               rowAddr1[0x00] = src_texel[16];  rowAddr1[0x01] = src_texel[17];
               rowAddr1[0x02] = src_texel[18];  rowAddr1[0x03] = src_texel[19];
               rowAddr1[0x10] = src_texel[20];  rowAddr1[0x11] = src_texel[21];
               rowAddr1[0x12] = src_texel[22];  rowAddr1[0x13] = src_texel[23];
               rowAddr1[0x20] = src_texel[24];  rowAddr1[0x21] = src_texel[25];
               rowAddr1[0x22] = src_texel[26];  rowAddr1[0x23] = src_texel[27];
               rowAddr1[0x30] = src_texel[28];  rowAddr1[0x31] = src_texel[29];
               rowAddr1[0x32] = src_texel[30];  rowAddr1[0x33] = src_texel[31];
#endif
            }
            else
            {
               /* Slower path for edges of image */
               uint32_t   endSubX1 = endSubX < (SUB_TILE_WIDTH - 1) ? endSubX : SUB_TILE_WIDTH - 1;

               for (subX = startSubX; subX <= endSubX1; ++subX)
               {
                  uint32_t *dst_texel =  rowAddr0 + XSWIZZLE32(subX);
                  *dst_texel = *src_texel;
                  src_texel += 1;
               }

               for (subX = startSubX > SUB_TILE_WIDTH ? startSubX : SUB_TILE_WIDTH; subX <= endSubX; ++subX)
               {
                  uint32_t *dst_texel =  rowAddr1 + XSWIZZLE32(subX);
                  *dst_texel = *src_texel;
                  src_texel += 1;
               }
            }
         }
      }
   }

   return true;
}

/* khrn_copy_YUYV_to_tf32
 *
 * Copies an array in YUYV 16-bit linear format to the equivalent 32-bit Tformat array
 */
static bool OPTIMIZE_FN khrn_copy_YUYV_to_tf32(
   uint8_t   *dst,                             /* address of Tformat image                                             */
   uint32_t  dstWidth,                         /* width of destination in texels                                       */
   uint32_t  dstStartX, uint32_t dstStartY,    /* bottom left of destination region                                    */
   uint32_t  width,     uint32_t height,       /* top right of destination region                                      */
   uint8_t   *src,                             /* address of linear image                                              */
   uint32_t  srcStartX, uint32_t srcStartY,    /* bottom left of source region                                         */
   uint32_t  srcStride,                        /* stride of source image in bytes (i.e. line length including padding) */
   int32_t   srcDir)
{
   static const uint32_t   TILE_XSHIFT       = 5;
   static const uint32_t   TILE_XBITS        = 0x1f;
   static const uint32_t   TILE_YSHIFT       = 5;
   static const uint32_t   TILE_YBITS        = 0x1f;
   static const uint32_t   SUB_TILE_WIDTH    = 16;
   static const uint32_t   TEXEL_SIZE        = 2;     /* Source data is 16-bpp */
   static const uint32_t   SRC_TILE_STRIDE32 = 16;

   uint32_t   blockY, blockX, subY, subX;

   uint32_t   dstBlockWidth  = ((dstWidth + TILE_XBITS) & ~TILE_XBITS) >> TILE_XSHIFT;  /* Width in 4k tiles */
   uint32_t   dstBlockStride = dstBlockWidth * TILE_SIZE;

   uint32_t   dstEndX    = dstStartX + width - 1;
   uint32_t   dstEndY    = dstStartY + height - 1;

   uint32_t   startBlockX = dstStartX >> TILE_XSHIFT;
   uint32_t   startBlockY = dstStartY >> TILE_YSHIFT;
   uint32_t   endBlockX   = dstEndX   >> TILE_XSHIFT;
   uint32_t   endBlockY   = dstEndY   >> TILE_YSHIFT;

   int   offX = srcStartX - dstStartX;
   int   offY = srcStartY - dstStartY;

#ifdef __ARM_NEON__
    uint8x8_t swizzle = vcreate_u8(0x8003010280030100ULL);
#endif

   /* Doesn't support flipped order */
   if (srcDir != 1)
      return false;

   /* For each 4k tile vertically */
   for (blockY = startBlockY; blockY <= endBlockY; ++blockY)
   {
      uint32_t   startSubY = blockY == startBlockY ? (dstStartY & TILE_YBITS) : 0;
      uint32_t   endSubY   = blockY == endBlockY   ? (dstEndY   & TILE_YBITS) : TILE_YBITS;

      uint32_t   odd = blockY & 1;
      uint32_t   mask = odd ? MAP_ODD : MAP_EVEN;

      /* For each 4k tile horizontally */
      for (blockX = startBlockX; blockX <= endBlockX; ++blockX)
      {
         uint32_t   startSubX = blockX == startBlockX ? (dstStartX & TILE_XBITS) : 0;
         uint32_t   endSubX   = blockX == endBlockX   ? (dstEndX   & TILE_XBITS) : TILE_XBITS;
         bool       fast      = startSubX == 0 && endSubX == TILE_XBITS;

         uint32_t   blockXR   = odd ? dstBlockWidth - blockX - 1 : blockX;

         /* Address of tile */
         uint8_t     *tileAddr  = dst + blockY * dstBlockStride + blockXR * TILE_SIZE;
         uint32_t    Y = (blockY << TILE_YSHIFT) + offY;
         uint32_t    X = (blockX << TILE_XSHIFT) + startSubX + offX;

         /* For each row in the tile */
         for (subY = startSubY; subY <= endSubY; ++subY)
         {
            uint32_t  *src_texel = (uint32_t *)(src + (Y + subY) * srcStride + X * TEXEL_SIZE);

            uint32_t  offset     = mask >> ((subY & 0x10) >> 1);
            uint8_t   *rowAddr   = tileAddr + YSWIZZLE32(subY);
            uint32_t  *rowAddr0  = (uint32_t *)(rowAddr + ((offset & 0x0f) << 8));
            uint32_t  *rowAddr1  = (uint32_t *)(rowAddr + ((offset & 0xf0) << 4));
            uint32_t  twoTexelsA, uv;
#ifndef __ARM_NEON__
            uint32_t  twoTexelsB;
#endif
            if (fast)
            {
#ifdef __ARM_NEON__
               uint32x2_t twoTexelsA, twoTexelsB, twoTexelsC, twoTexelsD;
               uint32x4_t res0, res1;

               twoTexelsA = vdup_n_u32(0);
               twoTexelsB = vdup_n_u32(0);
               twoTexelsC = vdup_n_u32(0);
               twoTexelsD = vdup_n_u32(0);

               fetch(&src_texel[SRC_TILE_STRIDE32]);

               /* batch 0 */
               twoTexelsA = vld1_lane_u32(&src_texel[0], twoTexelsA, 0);
               twoTexelsB = vld1_lane_u32(&src_texel[1], twoTexelsB, 0);
               twoTexelsC = vld1_lane_u32(&src_texel[2], twoTexelsC, 0);
               twoTexelsD = vld1_lane_u32(&src_texel[3], twoTexelsD, 0);

               res0 = vcombine_u32(vreinterpret_u32_u8(vtbl1_u8(vreinterpret_u8_u32(twoTexelsA), swizzle)),
                                   vreinterpret_u32_u8(vtbl1_u8(vreinterpret_u8_u32(twoTexelsB), swizzle)));
               res1 = vcombine_u32(vreinterpret_u32_u8(vtbl1_u8(vreinterpret_u8_u32(twoTexelsC), swizzle)),
                                   vreinterpret_u32_u8(vtbl1_u8(vreinterpret_u8_u32(twoTexelsD), swizzle)));

               vst1q_u32(&rowAddr0[0x00], res0);
               vst1q_u32(&rowAddr0[0x10], res1);

               /* batch 1 */
               twoTexelsA = vld1_lane_u32(&src_texel[4], twoTexelsA, 0);
               twoTexelsB = vld1_lane_u32(&src_texel[5], twoTexelsB, 0);
               twoTexelsC = vld1_lane_u32(&src_texel[6], twoTexelsC, 0);
               twoTexelsD = vld1_lane_u32(&src_texel[7], twoTexelsD, 0);

               res0 = vcombine_u32(vreinterpret_u32_u8(vtbl1_u8(vreinterpret_u8_u32(twoTexelsA), swizzle)),
                                   vreinterpret_u32_u8(vtbl1_u8(vreinterpret_u8_u32(twoTexelsB), swizzle)));
               res1 = vcombine_u32(vreinterpret_u32_u8(vtbl1_u8(vreinterpret_u8_u32(twoTexelsC), swizzle)),
                                   vreinterpret_u32_u8(vtbl1_u8(vreinterpret_u8_u32(twoTexelsD), swizzle)));

               vst1q_u32(&rowAddr0[0x20], res0);
               vst1q_u32(&rowAddr0[0x30], res1);

               /* batch 2 */
               twoTexelsA = vld1_lane_u32(&src_texel[8], twoTexelsA, 0);
               twoTexelsB = vld1_lane_u32(&src_texel[9], twoTexelsB, 0);
               twoTexelsC = vld1_lane_u32(&src_texel[10], twoTexelsC, 0);
               twoTexelsD = vld1_lane_u32(&src_texel[11], twoTexelsD, 0);

               res0 = vcombine_u32(vreinterpret_u32_u8(vtbl1_u8(vreinterpret_u8_u32(twoTexelsA), swizzle)),
                                   vreinterpret_u32_u8(vtbl1_u8(vreinterpret_u8_u32(twoTexelsB), swizzle)));
               res1 = vcombine_u32(vreinterpret_u32_u8(vtbl1_u8(vreinterpret_u8_u32(twoTexelsC), swizzle)),
                                   vreinterpret_u32_u8(vtbl1_u8(vreinterpret_u8_u32(twoTexelsD), swizzle)));

               vst1q_u32(&rowAddr1[0x00], res0);
               vst1q_u32(&rowAddr1[0x10], res1);

               /* batch 3 */
               twoTexelsA = vld1_lane_u32(&src_texel[12], twoTexelsA, 0);
               twoTexelsB = vld1_lane_u32(&src_texel[13], twoTexelsB, 0);
               twoTexelsC = vld1_lane_u32(&src_texel[14], twoTexelsC, 0);
               twoTexelsD = vld1_lane_u32(&src_texel[15], twoTexelsD, 0);

               res0 = vcombine_u32(vreinterpret_u32_u8(vtbl1_u8(vreinterpret_u8_u32(twoTexelsA), swizzle)),
                                   vreinterpret_u32_u8(vtbl1_u8(vreinterpret_u8_u32(twoTexelsB), swizzle)));
               res1 = vcombine_u32(vreinterpret_u32_u8(vtbl1_u8(vreinterpret_u8_u32(twoTexelsC), swizzle)),
                                   vreinterpret_u32_u8(vtbl1_u8(vreinterpret_u8_u32(twoTexelsD), swizzle)));

               vst1q_u32(&rowAddr1[0x20], res0);
               vst1q_u32(&rowAddr1[0x30], res1);
#else
               fetch(&src_texel[SRC_TILE_STRIDE32]);

               twoTexelsA = src_texel[0]; twoTexelsB = src_texel[1];
               uv = UV32(twoTexelsA); rowAddr0[0x00] = uv | Y132(twoTexelsA); rowAddr0[0x01] = uv | Y232(twoTexelsA);
               uv = UV32(twoTexelsB); rowAddr0[0x02] = uv | Y132(twoTexelsB); rowAddr0[0x03] = uv | Y232(twoTexelsB);
               twoTexelsA = src_texel[2]; twoTexelsB = src_texel[3];
               uv = UV32(twoTexelsA); rowAddr0[0x10] = uv | Y132(twoTexelsA); rowAddr0[0x11] = uv | Y232(twoTexelsA);
               uv = UV32(twoTexelsB); rowAddr0[0x12] = uv | Y132(twoTexelsB); rowAddr0[0x13] = uv | Y232(twoTexelsB);
               twoTexelsA = src_texel[4]; twoTexelsB = src_texel[5];
               uv = UV32(twoTexelsA); rowAddr0[0x20] = uv | Y132(twoTexelsA); rowAddr0[0x21] = uv | Y232(twoTexelsA);
               uv = UV32(twoTexelsB); rowAddr0[0x22] = uv | Y132(twoTexelsB); rowAddr0[0x23] = uv | Y232(twoTexelsB);
               twoTexelsA = src_texel[6]; twoTexelsB = src_texel[7];
               uv = UV32(twoTexelsA); rowAddr0[0x30] = uv | Y132(twoTexelsA); rowAddr0[0x31] = uv | Y232(twoTexelsA);
               uv = UV32(twoTexelsB); rowAddr0[0x32] = uv | Y132(twoTexelsB); rowAddr0[0x33] = uv | Y232(twoTexelsB);
               twoTexelsA = src_texel[8]; twoTexelsB = src_texel[9];
               uv = UV32(twoTexelsA); rowAddr1[0x00] = uv | Y132(twoTexelsA); rowAddr1[0x01] = uv | Y232(twoTexelsA);
               uv = UV32(twoTexelsB); rowAddr1[0x02] = uv | Y132(twoTexelsB); rowAddr1[0x03] = uv | Y232(twoTexelsB);
               twoTexelsA = src_texel[10]; twoTexelsB = src_texel[11];
               uv = UV32(twoTexelsA); rowAddr1[0x10] = uv | Y132(twoTexelsA); rowAddr1[0x11] = uv | Y232(twoTexelsA);
               uv = UV32(twoTexelsB); rowAddr1[0x12] = uv | Y132(twoTexelsB); rowAddr1[0x13] = uv | Y232(twoTexelsB);
               twoTexelsA = src_texel[12]; twoTexelsB = src_texel[13];
               uv = UV32(twoTexelsA); rowAddr1[0x20] = uv | Y132(twoTexelsA); rowAddr1[0x21] = uv | Y232(twoTexelsA);
               uv = UV32(twoTexelsB); rowAddr1[0x22] = uv | Y132(twoTexelsB); rowAddr1[0x23] = uv | Y232(twoTexelsB);
               twoTexelsA = src_texel[14]; twoTexelsB = src_texel[15];
               uv = UV32(twoTexelsA); rowAddr1[0x30] = uv | Y132(twoTexelsA); rowAddr1[0x31] = uv | Y232(twoTexelsA);
               uv = UV32(twoTexelsB); rowAddr1[0x32] = uv | Y132(twoTexelsB); rowAddr1[0x33] = uv | Y232(twoTexelsB);
#endif
            }
            else
            {
               /* Slower path for edges of image */
               uint32_t   endSubX1 = endSubX < (SUB_TILE_WIDTH - 1) ? endSubX : SUB_TILE_WIDTH - 1;

               for (subX = startSubX; subX <= endSubX1; subX += 2)
               {
                  uint32_t *dst_texel = rowAddr0 + XSWIZZLE32(subX);
                  twoTexelsA = *src_texel;

                  uv = UV32(twoTexelsA);
                  *dst_texel = uv | Y132(twoTexelsA);

                  dst_texel = rowAddr0 + XSWIZZLE32(subX + 1);
                  *dst_texel = uv | Y232(twoTexelsA);

                  src_texel += 1;
               }

               for (subX = startSubX > SUB_TILE_WIDTH ? startSubX : SUB_TILE_WIDTH; subX <= endSubX; subX += 2)
               {
                  uint32_t *dst_texel = rowAddr1 + XSWIZZLE32(subX);
                  twoTexelsA = *src_texel;

                  uv = UV32(twoTexelsA);
                  *dst_texel = uv | Y132(twoTexelsA);

                  dst_texel = rowAddr1 + XSWIZZLE32(subX + 1);
                  *dst_texel = uv | Y232(twoTexelsA);

                  src_texel += 1;
               }
            }
         }
      }
   }

   return true;
}

/* khrn_copy_YUYV_REV_to_tf32
 *
 * Copies an array in YUYV REV 16-bit linear format to the equivalent 32-bit Tformat array
 */
static bool OPTIMIZE_FN khrn_copy_YUYV_REV_to_tf32(
   uint8_t   *dst,                             /* address of Tformat image                                             */
   uint32_t  dstWidth,                         /* width of destination in texels                                       */
   uint32_t  dstStartX, uint32_t dstStartY,    /* bottom left of destination region                                    */
   uint32_t  width,     uint32_t height,       /* top right of destination region                                      */
   uint8_t   *src,                             /* address of linear image                                              */
   uint32_t  srcStartX, uint32_t srcStartY,    /* bottom left of source region                                         */
   uint32_t  srcStride,                        /* stride of source image in bytes (i.e. line length including padding) */
   int32_t   srcDir)
{
   static const uint32_t   TILE_XSHIFT       = 5;
   static const uint32_t   TILE_XBITS        = 0x1f;
   static const uint32_t   TILE_YSHIFT       = 5;
   static const uint32_t   TILE_YBITS        = 0x1f;
   static const uint32_t   SUB_TILE_WIDTH    = 16;
   static const uint32_t   TEXEL_SIZE        = 2;     /* Source data is 16-bpp */
   static const uint32_t   SRC_TILE_STRIDE32 = 16;

   uint32_t   blockY, blockX, subY, subX;

   uint32_t   dstBlockWidth  = ((dstWidth + TILE_XBITS) & ~TILE_XBITS) >> TILE_XSHIFT;  /* Width in 4k tiles */
   uint32_t   dstBlockStride = dstBlockWidth * TILE_SIZE;

   uint32_t   dstEndX    = dstStartX + width - 1;
   uint32_t   dstEndY    = dstStartY + height - 1;

   uint32_t   startBlockX = dstStartX >> TILE_XSHIFT;
   uint32_t   startBlockY = dstStartY >> TILE_YSHIFT;
   uint32_t   endBlockX   = dstEndX   >> TILE_XSHIFT;
   uint32_t   endBlockY   = dstEndY   >> TILE_YSHIFT;

   int   offX = srcStartX - dstStartX;
   int   offY = srcStartY - dstStartY;

#ifdef __ARM_NEON__
    uint8x8_t swizzle = vcreate_u8(0x8002000180020003ULL);
#endif

   /* Doesn't support flipped order */
   if (srcDir != 1)
      return false;

   /* For each 4k tile vertically */
   for (blockY = startBlockY; blockY <= endBlockY; ++blockY)
   {
      uint32_t   startSubY = blockY == startBlockY ? (dstStartY & TILE_YBITS) : 0;
      uint32_t   endSubY   = blockY == endBlockY   ? (dstEndY   & TILE_YBITS) : TILE_YBITS;

      uint32_t   odd = blockY & 1;
      uint32_t   mask = odd ? MAP_ODD : MAP_EVEN;

      /* For each 4k tile horizontally */
      for (blockX = startBlockX; blockX <= endBlockX; ++blockX)
      {
         uint32_t   startSubX = blockX == startBlockX ? (dstStartX & TILE_XBITS) : 0;
         uint32_t   endSubX   = blockX == endBlockX   ? (dstEndX   & TILE_XBITS) : TILE_XBITS;
         bool       fast      = startSubX == 0 && endSubX == TILE_XBITS;

         uint32_t   blockXR   = odd ? dstBlockWidth - blockX - 1 : blockX;

         /* Address of tile */
         uint8_t     *tileAddr  = dst + blockY * dstBlockStride + blockXR * TILE_SIZE;
         uint32_t    Y = (blockY << TILE_YSHIFT) + offY;
         uint32_t    X = (blockX << TILE_XSHIFT) + startSubX + offX;

         /* For each row in the tile */
         for (subY = startSubY; subY <= endSubY; ++subY)
         {
            uint32_t  *src_texel = (uint32_t *)(src + (Y + subY) * srcStride + X * TEXEL_SIZE);

            uint32_t  offset     = mask >> ((subY & 0x10) >> 1);
            uint8_t   *rowAddr   = tileAddr + YSWIZZLE32(subY);
            uint32_t  *rowAddr0  = (uint32_t *)(rowAddr + ((offset & 0x0f) << 8));
            uint32_t  *rowAddr1  = (uint32_t *)(rowAddr + ((offset & 0xf0) << 4));
            uint32_t  twoTexelsA, uv;
#ifndef __ARM_NEON__
            uint32_t  twoTexelsB;
#endif
            if (fast)
            {
#ifdef __ARM_NEON__
                uint32x2_t twoTexelsA, twoTexelsB, twoTexelsC, twoTexelsD;
                uint32x4_t res0, res1;

                twoTexelsA = vdup_n_u32(0);
                twoTexelsB = vdup_n_u32(0);
                twoTexelsC = vdup_n_u32(0);
                twoTexelsD = vdup_n_u32(0);

                fetch(&src_texel[SRC_TILE_STRIDE32]);

                /* batch 0 */
                twoTexelsA = vld1_lane_u32(&src_texel[0], twoTexelsA, 0);
                twoTexelsB = vld1_lane_u32(&src_texel[1], twoTexelsB, 0);
                twoTexelsC = vld1_lane_u32(&src_texel[2], twoTexelsC, 0);
                twoTexelsD = vld1_lane_u32(&src_texel[3], twoTexelsD, 0);

                res0 = vcombine_u32(vreinterpret_u32_u8(vtbl1_u8(vreinterpret_u8_u32(twoTexelsA), swizzle)),
                                    vreinterpret_u32_u8(vtbl1_u8(vreinterpret_u8_u32(twoTexelsB), swizzle)));
                res1 = vcombine_u32(vreinterpret_u32_u8(vtbl1_u8(vreinterpret_u8_u32(twoTexelsC), swizzle)),
                                    vreinterpret_u32_u8(vtbl1_u8(vreinterpret_u8_u32(twoTexelsD), swizzle)));

                vst1q_u32(&rowAddr0[0x00], res0);
                vst1q_u32(&rowAddr0[0x10], res1);

                /* batch 1 */
                twoTexelsA = vld1_lane_u32(&src_texel[4], twoTexelsA, 0);
                twoTexelsB = vld1_lane_u32(&src_texel[5], twoTexelsB, 0);
                twoTexelsC = vld1_lane_u32(&src_texel[6], twoTexelsC, 0);
                twoTexelsD = vld1_lane_u32(&src_texel[7], twoTexelsD, 0);

                res0 = vcombine_u32(vreinterpret_u32_u8(vtbl1_u8(vreinterpret_u8_u32(twoTexelsA), swizzle)),
                                    vreinterpret_u32_u8(vtbl1_u8(vreinterpret_u8_u32(twoTexelsB), swizzle)));
                res1 = vcombine_u32(vreinterpret_u32_u8(vtbl1_u8(vreinterpret_u8_u32(twoTexelsC), swizzle)),
                                    vreinterpret_u32_u8(vtbl1_u8(vreinterpret_u8_u32(twoTexelsD), swizzle)));

                vst1q_u32(&rowAddr0[0x20], res0);
                vst1q_u32(&rowAddr0[0x30], res1);

                /* batch 2 */
                twoTexelsA = vld1_lane_u32(&src_texel[8], twoTexelsA, 0);
                twoTexelsB = vld1_lane_u32(&src_texel[9], twoTexelsB, 0);
                twoTexelsC = vld1_lane_u32(&src_texel[10], twoTexelsC, 0);
                twoTexelsD = vld1_lane_u32(&src_texel[11], twoTexelsD, 0);

                res0 = vcombine_u32(vreinterpret_u32_u8(vtbl1_u8(vreinterpret_u8_u32(twoTexelsA), swizzle)),
                                    vreinterpret_u32_u8(vtbl1_u8(vreinterpret_u8_u32(twoTexelsB), swizzle)));
                res1 = vcombine_u32(vreinterpret_u32_u8(vtbl1_u8(vreinterpret_u8_u32(twoTexelsC), swizzle)),
                                    vreinterpret_u32_u8(vtbl1_u8(vreinterpret_u8_u32(twoTexelsD), swizzle)));

                vst1q_u32(&rowAddr1[0x00], res0);
                vst1q_u32(&rowAddr1[0x10], res1);

                /* batch 3 */
                twoTexelsA = vld1_lane_u32(&src_texel[12], twoTexelsA, 0);
                twoTexelsB = vld1_lane_u32(&src_texel[13], twoTexelsB, 0);
                twoTexelsC = vld1_lane_u32(&src_texel[14], twoTexelsC, 0);
                twoTexelsD = vld1_lane_u32(&src_texel[15], twoTexelsD, 0);

                res0 = vcombine_u32(vreinterpret_u32_u8(vtbl1_u8(vreinterpret_u8_u32(twoTexelsA), swizzle)),
                                    vreinterpret_u32_u8(vtbl1_u8(vreinterpret_u8_u32(twoTexelsB), swizzle)));
                res1 = vcombine_u32(vreinterpret_u32_u8(vtbl1_u8(vreinterpret_u8_u32(twoTexelsC), swizzle)),
                                    vreinterpret_u32_u8(vtbl1_u8(vreinterpret_u8_u32(twoTexelsD), swizzle)));

                vst1q_u32(&rowAddr1[0x20], res0);
                vst1q_u32(&rowAddr1[0x30], res1);
#else
               fetch(&src_texel[SRC_TILE_STRIDE32]);

               twoTexelsA = src_texel[0]; twoTexelsB = src_texel[1];
               uv = UV32_REV(twoTexelsA); rowAddr0[0x00] = uv | Y132_REV(twoTexelsA); rowAddr0[0x01] = uv | Y232_REV(twoTexelsA);
               uv = UV32_REV(twoTexelsB); rowAddr0[0x02] = uv | Y132_REV(twoTexelsB); rowAddr0[0x03] = uv | Y232_REV(twoTexelsB);
               twoTexelsA = src_texel[2]; twoTexelsB = src_texel[3];
               uv = UV32_REV(twoTexelsA); rowAddr0[0x10] = uv | Y132_REV(twoTexelsA); rowAddr0[0x11] = uv | Y232_REV(twoTexelsA);
               uv = UV32_REV(twoTexelsB); rowAddr0[0x12] = uv | Y132_REV(twoTexelsB); rowAddr0[0x13] = uv | Y232_REV(twoTexelsB);
               twoTexelsA = src_texel[4]; twoTexelsB = src_texel[5];
               uv = UV32_REV(twoTexelsA); rowAddr0[0x20] = uv | Y132_REV(twoTexelsA); rowAddr0[0x21] = uv | Y232_REV(twoTexelsA);
               uv = UV32_REV(twoTexelsB); rowAddr0[0x22] = uv | Y132_REV(twoTexelsB); rowAddr0[0x23] = uv | Y232_REV(twoTexelsB);
               twoTexelsA = src_texel[6]; twoTexelsB = src_texel[7];
               uv = UV32_REV(twoTexelsA); rowAddr0[0x30] = uv | Y132_REV(twoTexelsA); rowAddr0[0x31] = uv | Y232_REV(twoTexelsA);
               uv = UV32_REV(twoTexelsB); rowAddr0[0x32] = uv | Y132_REV(twoTexelsB); rowAddr0[0x33] = uv | Y232_REV(twoTexelsB);
               twoTexelsA = src_texel[8]; twoTexelsB = src_texel[9];
               uv = UV32_REV(twoTexelsA); rowAddr1[0x00] = uv | Y132_REV(twoTexelsA); rowAddr1[0x01] = uv | Y232_REV(twoTexelsA);
               uv = UV32_REV(twoTexelsB); rowAddr1[0x02] = uv | Y132_REV(twoTexelsB); rowAddr1[0x03] = uv | Y232_REV(twoTexelsB);
               twoTexelsA = src_texel[10]; twoTexelsB = src_texel[11];
               uv = UV32_REV(twoTexelsA); rowAddr1[0x10] = uv | Y132_REV(twoTexelsA); rowAddr1[0x11] = uv | Y232_REV(twoTexelsA);
               uv = UV32_REV(twoTexelsB); rowAddr1[0x12] = uv | Y132_REV(twoTexelsB); rowAddr1[0x13] = uv | Y232_REV(twoTexelsB);
               twoTexelsA = src_texel[12]; twoTexelsB = src_texel[13];
               uv = UV32_REV(twoTexelsA); rowAddr1[0x20] = uv | Y132_REV(twoTexelsA); rowAddr1[0x21] = uv | Y232_REV(twoTexelsA);
               uv = UV32_REV(twoTexelsB); rowAddr1[0x22] = uv | Y132_REV(twoTexelsB); rowAddr1[0x23] = uv | Y232_REV(twoTexelsB);
               twoTexelsA = src_texel[14]; twoTexelsB = src_texel[15];
               uv = UV32_REV(twoTexelsA); rowAddr1[0x30] = uv | Y132_REV(twoTexelsA); rowAddr1[0x31] = uv | Y232_REV(twoTexelsA);
               uv = UV32_REV(twoTexelsB); rowAddr1[0x32] = uv | Y132_REV(twoTexelsB); rowAddr1[0x33] = uv | Y232_REV(twoTexelsB);
#endif
            }
            else
            {
               /* Slower path for edges of image */
               uint32_t   endSubX1 = endSubX < (SUB_TILE_WIDTH - 1) ? endSubX : SUB_TILE_WIDTH - 1;

               for (subX = startSubX; subX <= endSubX1; subX += 2)
               {
                  uint32_t *dst_texel = rowAddr0 + XSWIZZLE32(subX);
                  twoTexelsA = *src_texel;

                  uv = UV32_REV(twoTexelsA);
                  *dst_texel = uv | Y132_REV(twoTexelsA);

                  dst_texel = rowAddr0 + XSWIZZLE32(subX + 1);
                  *dst_texel = uv | Y232_REV(twoTexelsA);

                  src_texel += 1;
               }

               for (subX = startSubX > SUB_TILE_WIDTH ? startSubX : SUB_TILE_WIDTH; subX <= endSubX; subX += 2)
               {
                  uint32_t *dst_texel = rowAddr1 + XSWIZZLE32(subX);
                  twoTexelsA = *src_texel;

                  uv = UV32_REV(twoTexelsA);
                  *dst_texel = uv | Y132_REV(twoTexelsA);

                  dst_texel = rowAddr1 + XSWIZZLE32(subX + 1);
                  *dst_texel = uv | Y232_REV(twoTexelsA);

                  src_texel += 1;
               }
            }
         }
      }
   }

   return true;
}

/* khrn_copy_YUYV_to_rso
 *
 * Copies an array in YUYV 16-bit linear format to the equivalent 16-bit rso array
 */
static bool khrn_copy_YUYV_to_rso(
   uint8_t   *dst,                             /* address of rso image                                                 */
   uint32_t  dstWidth,                         /* width of destination in texels                                       */
   uint32_t  dstStartX, uint32_t dstStartY,    /* bottom left of destination region                                    */
   uint32_t  width,     uint32_t height,       /* top right of destination region                                      */
   uint8_t   *src,                             /* address of linear image                                              */
   uint32_t  srcStartX, uint32_t srcStartY,    /* bottom left of source region                                         */
   uint32_t  srcStride,                        /* stride of source image in bytes (i.e. line length including padding) */
   int32_t   srcDir)
{
   uint32_t y;

   if (srcDir != 1)
      return false;

   for (y = 0; y < height; y++)
   {
      uint8_t *dstRow = dst + ((dstStartY + y) * dstWidth * 2 + (dstStartX * 2));
      uint8_t *srcRow = src + ((srcStartY + y) * srcStride + (srcStartX * 2));

      khrn_memcpy(dstRow, srcRow, width * 2);
   }

   return true;
}

#ifdef BIG_ENDIAN_CPU

#define PIX0(A)         (A)
#define PIX1(A, B)      (A) << 24 | (B) >> 8
#define PIX2(B, C)      (B) << 16 | (C) >> 16
#define PIX3(C)         (C) << 8
#define PIXEL(R, G, B)  (R) << 24 | (G) << 16 | (B) << 8

#else

#define PIX0(A)         (A)
#define PIX1(A, B)      (A) >> 24 | (B) << 8
#define PIX2(B, C)      (B) >> 16 | (C) << 16
#define PIX3(C)         (C) >> 8

#define PIXEL(R, G, B)  (R) | (G) << 8 | (B) << 16

#endif

#ifdef __ARM_NEON__
#define uint32x4_to_8x8x2(v) ((uint8x8x2_t) { vget_low_u8(vreinterpretq_u8_u32(v)), vget_high_u8(vreinterpretq_u8_u32(v)) })
#endif

/* khrn_copy_888_to_tf32
 *
 * Copies an array in RGB 24-bit linear format to the equivalent 32-bit Tformat array
 */
static bool OPTIMIZE_FN khrn_copy_888_to_tf32(
   uint8_t   *dst,                             /* address of Tformat image                                               */
   uint32_t  dstWidth,                         /* width of destination in texels                                         */
   uint32_t  dstStartX, uint32_t dstStartY,    /* bottom left of destination region                                      */
   uint32_t  width,     uint32_t height,       /* top right of destination region                                        */
   uint8_t   *src,                             /* address of linear image                                                */
   uint32_t  srcStartX, uint32_t srcStartY,    /* bottom left of source region                                           */
   uint32_t  srcStride,                        /* stride of source image in bytes (i.e. line length including padding)   */
   int32_t   srcDir)
{
   static const uint32_t   TILE_XSHIFT        = 5;
   static const uint32_t   TILE_XBITS         = 0x1f;
   static const uint32_t   TILE_YSHIFT        = 5;
   static const uint32_t   TILE_YBITS         = 0x1f;
   static const uint32_t   SUB_TILE_WIDTH     = 16;
   static const uint32_t   TEXEL_SIZE         = 3;
   static const uint32_t   SRC_TILE_STRIDE32  = 24;
   static const uint32_t   SRC_STRIDE32       = 12;

   uint32_t   srcStride32 = srcStride / 4;
   uint32_t   blockY, blockX, subY, subX;

   uint32_t   dstBlockWidth  = ((dstWidth + TILE_XBITS) & ~TILE_XBITS) >> TILE_XSHIFT;  /* Width in 4k tiles */
   uint32_t   dstBlockStride = dstBlockWidth * TILE_SIZE;

   uint32_t   dstEndX    = dstStartX + width - 1;
   uint32_t   dstEndY    = dstStartY + height - 1;

   uint32_t   startBlockX = dstStartX >> TILE_XSHIFT;
   uint32_t   startBlockY = dstStartY >> TILE_YSHIFT;
   uint32_t   endBlockX   = dstEndX   >> TILE_XSHIFT;
   uint32_t   endBlockY   = dstEndY   >> TILE_YSHIFT;

   int   offX = srcStartX - dstStartX;
   int   offY = srcStartY - dstStartY;

#ifdef __ARM_NEON__
   uint8x16_t swizzle = vcombine_u8(vcreate_u8(0x8005040380020100ULL),
                                    vcreate_u8(0x800b0a0980080706ULL)
                                    );
#endif

   if (srcDir != 1)
      return false;

   /* For each 4k block vertically */
   for (blockY = startBlockY; blockY <= endBlockY; ++blockY)
   {
      uint32_t startSubY = blockY == startBlockY ? (dstStartY & TILE_YBITS) : 0;
      uint32_t endSubY   = blockY == endBlockY   ? (dstEndY   & TILE_YBITS) : TILE_YBITS;

      uint32_t   odd  = blockY & 1;
      uint32_t   mask = odd ? MAP_ODD : MAP_EVEN;

      /* For each 4k block horizontally */
      for (blockX = startBlockX; blockX <= endBlockX; ++blockX)
      {
         uint32_t   startSubX = blockX == startBlockX ? (dstStartX & TILE_XBITS) : 0;
         uint32_t   endSubX   = blockX == endBlockX   ? (dstEndX   & TILE_XBITS) : TILE_XBITS;
         bool       fast      = offX == 0 && startSubX == 0 && endSubX == TILE_XBITS;

         uint32_t   blockXR   = odd ? dstBlockWidth - blockX - 1 : blockX;

         /* Address of tile */
         uint8_t     *tileAddr  = dst + blockY * dstBlockStride + blockXR * TILE_SIZE;
         uint32_t    Y = (blockY << TILE_YSHIFT) + offY;
         uint32_t    X = (blockX << TILE_XSHIFT) + startSubX + offX;

         /* For each row in the tile */
         for (subY = startSubY; subY <= endSubY; ++subY)
         {
            uint8_t   *src_texel_8 = src + (Y + subY) * srcStride + X * TEXEL_SIZE;

            uint32_t  offset     = mask >> ((subY & 0x10) >> 1);
            uint8_t   *rowAddr   = tileAddr + YSWIZZLE32(subY);
            uint32_t  *rowAddr0  = (uint32_t *)(rowAddr + ((offset & 0x0f) << 8));
            uint32_t  *rowAddr1  = (uint32_t *)(rowAddr + ((offset & 0xf0) << 4));
            bool      extra_fast = fast && ((subY & 0x3) == 0) && ((subY + 0x3) <= endSubY);

            if (extra_fast)
            {
               /* This case copies four lines at a time, and writes a whole destination cache line at a time
                  which allows the use of "prepare for store" prefetch instructions eliminating the need to
                  load data into the cache for immediate overwrite */
               uint32_t *src_texel = (uint32_t *)src_texel_8;
               uint32_t *src0 = src_texel;
               uint32_t *src1 = src0 + srcStride32;
               uint32_t *src2 = src1 + srcStride32;
               uint32_t *src3 = src2 + srcStride32;
               uint32_t *dest = rowAddr0;
               uint32_t subtile;

               /* Prefetches the data for the start of the next tile */
               fetch(&src0[SRC_TILE_STRIDE32]); fetch(&src1[SRC_TILE_STRIDE32]);
               fetch(&src2[SRC_TILE_STRIDE32]); fetch(&src3[SRC_TILE_STRIDE32]);

               for (subtile = 0; subtile < 2; ++subtile)
               {
#ifdef __ARM_NEON__
                  uint32x4_t tmp0, tmp1;
                  tmp0 = vdupq_n_u32(0);
                  tmp1 = vdupq_n_u32(0);

                  /* missaligned, so separate loads */
                  tmp0 = vld1q_lane_u32(&src0[ 0], tmp0, 0);
                  tmp0 = vld1q_lane_u32(&src0[ 1], tmp0, 1);
                  tmp0 = vld1q_lane_u32(&src0[ 2], tmp0, 2);

                  tmp1 = vld1q_lane_u32(&src0[ 3], tmp1, 0);
                  tmp1 = vld1q_lane_u32(&src0[ 4], tmp1, 1);
                  tmp1 = vld1q_lane_u32(&src0[ 5], tmp1, 2);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-braces"
                  /* GCC incorrectly warns here, fixed in GCC 5.x */
                  tmp0 = vcombine_u32(vreinterpret_u32_u8(vtbl2_u8(uint32x4_to_8x8x2(tmp0), vget_low_u8(swizzle))),
                                      vreinterpret_u32_u8(vtbl2_u8(uint32x4_to_8x8x2(tmp0), vget_high_u8(swizzle))));
                  tmp1 = vcombine_u32(vreinterpret_u32_u8(vtbl2_u8(uint32x4_to_8x8x2(tmp1), vget_low_u8(swizzle))),
                                      vreinterpret_u32_u8(vtbl2_u8(uint32x4_to_8x8x2(tmp1), vget_high_u8(swizzle))));
#pragma GCC diagnostic pop

                  vst1q_u32(&dest[0x00], tmp0);
                  vst1q_u32(&dest[0x10], tmp1);

                  /* missaligned, so separate loads */
                  tmp0 = vld1q_lane_u32(&src1[ 0], tmp0, 0);
                  tmp0 = vld1q_lane_u32(&src1[ 1], tmp0, 1);
                  tmp0 = vld1q_lane_u32(&src1[ 2], tmp0, 2);

                  tmp1 = vld1q_lane_u32(&src1[ 3], tmp1, 0);
                  tmp1 = vld1q_lane_u32(&src1[ 4], tmp1, 1);
                  tmp1 = vld1q_lane_u32(&src1[ 5], tmp1, 2);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-braces"
                  /* GCC incorrectly warns here, fixed in GCC 5.x */
                  tmp0 = vcombine_u32(vreinterpret_u32_u8(vtbl2_u8(uint32x4_to_8x8x2(tmp0), vget_low_u8(swizzle))),
                                      vreinterpret_u32_u8(vtbl2_u8(uint32x4_to_8x8x2(tmp0), vget_high_u8(swizzle))));
                  tmp1 = vcombine_u32(vreinterpret_u32_u8(vtbl2_u8(uint32x4_to_8x8x2(tmp1), vget_low_u8(swizzle))),
                                      vreinterpret_u32_u8(vtbl2_u8(uint32x4_to_8x8x2(tmp1), vget_high_u8(swizzle))));
#pragma GCC diagnostic pop

                  vst1q_u32(&dest[0x04], tmp0);
                  vst1q_u32(&dest[0x14], tmp1);

                  /* missaligned, so separate loads */
                  tmp0 = vld1q_lane_u32(&src2[ 0], tmp0, 0);
                  tmp0 = vld1q_lane_u32(&src2[ 1], tmp0, 1);
                  tmp0 = vld1q_lane_u32(&src2[ 2], tmp0, 2);

                  tmp1 = vld1q_lane_u32(&src2[ 3], tmp1, 0);
                  tmp1 = vld1q_lane_u32(&src2[ 4], tmp1, 1);
                  tmp1 = vld1q_lane_u32(&src2[ 5], tmp1, 2);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-braces"
                  /* GCC incorrectly warns here, fixed in GCC 5.x */
                  tmp0 = vcombine_u32(vreinterpret_u32_u8(vtbl2_u8(uint32x4_to_8x8x2(tmp0), vget_low_u8(swizzle))),
                                      vreinterpret_u32_u8(vtbl2_u8(uint32x4_to_8x8x2(tmp0), vget_high_u8(swizzle))));
                  tmp1 = vcombine_u32(vreinterpret_u32_u8(vtbl2_u8(uint32x4_to_8x8x2(tmp1), vget_low_u8(swizzle))),
                                      vreinterpret_u32_u8(vtbl2_u8(uint32x4_to_8x8x2(tmp1), vget_high_u8(swizzle))));
#pragma GCC diagnostic pop

                  vst1q_u32(&dest[0x08], tmp0);
                  vst1q_u32(&dest[0x18], tmp1);

                  /* missaligned, so separate loads */
                  tmp0 = vld1q_lane_u32(&src3[ 0], tmp0, 0);
                  tmp0 = vld1q_lane_u32(&src3[ 1], tmp0, 1);
                  tmp0 = vld1q_lane_u32(&src3[ 2], tmp0, 2);

                  tmp1 = vld1q_lane_u32(&src3[ 3], tmp1, 0);
                  tmp1 = vld1q_lane_u32(&src3[ 4], tmp1, 1);
                  tmp1 = vld1q_lane_u32(&src3[ 5], tmp1, 2);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-braces"
                  /* GCC incorrectly warns here, fixed in GCC 5.x */
                  tmp0 = vcombine_u32(vreinterpret_u32_u8(vtbl2_u8(uint32x4_to_8x8x2(tmp0), vget_low_u8(swizzle))),
                                      vreinterpret_u32_u8(vtbl2_u8(uint32x4_to_8x8x2(tmp0), vget_high_u8(swizzle))));
                  tmp1 = vcombine_u32(vreinterpret_u32_u8(vtbl2_u8(uint32x4_to_8x8x2(tmp1), vget_low_u8(swizzle))),
                                      vreinterpret_u32_u8(vtbl2_u8(uint32x4_to_8x8x2(tmp1), vget_high_u8(swizzle))));
#pragma GCC diagnostic pop

                  vst1q_u32(&dest[0x0c], tmp0);
                  vst1q_u32(&dest[0x1c], tmp1);

                  /* missaligned, so separate loads */
                  tmp0 = vld1q_lane_u32(&src0[ 6], tmp0, 0);
                  tmp0 = vld1q_lane_u32(&src0[ 7], tmp0, 1);
                  tmp0 = vld1q_lane_u32(&src0[ 8], tmp0, 2);

                  tmp1 = vld1q_lane_u32(&src0[ 9], tmp1, 0);
                  tmp1 = vld1q_lane_u32(&src0[10], tmp1, 1);
                  tmp1 = vld1q_lane_u32(&src0[11], tmp1, 2);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-braces"
                  /* GCC incorrectly warns here, fixed in GCC 5.x */
                  tmp0 = vcombine_u32(vreinterpret_u32_u8(vtbl2_u8(uint32x4_to_8x8x2(tmp0), vget_low_u8(swizzle))),
                                      vreinterpret_u32_u8(vtbl2_u8(uint32x4_to_8x8x2(tmp0), vget_high_u8(swizzle))));
                  tmp1 = vcombine_u32(vreinterpret_u32_u8(vtbl2_u8(uint32x4_to_8x8x2(tmp1), vget_low_u8(swizzle))),
                                      vreinterpret_u32_u8(vtbl2_u8(uint32x4_to_8x8x2(tmp1), vget_high_u8(swizzle))));
#pragma GCC diagnostic pop

                  vst1q_u32(&dest[0x20], tmp0);
                  vst1q_u32(&dest[0x30], tmp1);

                  /* missaligned, so separate loads */
                  tmp0 = vld1q_lane_u32(&src1[ 6], tmp0, 0);
                  tmp0 = vld1q_lane_u32(&src1[ 7], tmp0, 1);
                  tmp0 = vld1q_lane_u32(&src1[ 8], tmp0, 2);

                  tmp1 = vld1q_lane_u32(&src1[ 9], tmp1, 0);
                  tmp1 = vld1q_lane_u32(&src1[10], tmp1, 1);
                  tmp1 = vld1q_lane_u32(&src1[11], tmp1, 2);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-braces"
                  /* GCC incorrectly warns here, fixed in GCC 5.x */
                  tmp0 = vcombine_u32(vreinterpret_u32_u8(vtbl2_u8(uint32x4_to_8x8x2(tmp0), vget_low_u8(swizzle))),
                                      vreinterpret_u32_u8(vtbl2_u8(uint32x4_to_8x8x2(tmp0), vget_high_u8(swizzle))));
                  tmp1 = vcombine_u32(vreinterpret_u32_u8(vtbl2_u8(uint32x4_to_8x8x2(tmp1), vget_low_u8(swizzle))),
                                      vreinterpret_u32_u8(vtbl2_u8(uint32x4_to_8x8x2(tmp1), vget_high_u8(swizzle))));
#pragma GCC diagnostic pop

                  vst1q_u32(&dest[0x24], tmp0);
                  vst1q_u32(&dest[0x34], tmp1);

                  /* missaligned, so separate loads */
                  tmp0 = vld1q_lane_u32(&src2[ 6], tmp0, 0);
                  tmp0 = vld1q_lane_u32(&src2[ 7], tmp0, 1);
                  tmp0 = vld1q_lane_u32(&src2[ 8], tmp0, 2);

                  tmp1 = vld1q_lane_u32(&src2[ 9], tmp1, 0);
                  tmp1 = vld1q_lane_u32(&src2[10], tmp1, 1);
                  tmp1 = vld1q_lane_u32(&src2[11], tmp1, 2);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-braces"
                  /* GCC incorrectly warns here, fixed in GCC 5.x */
                  tmp0 = vcombine_u32(vreinterpret_u32_u8(vtbl2_u8(uint32x4_to_8x8x2(tmp0), vget_low_u8(swizzle))),
                                      vreinterpret_u32_u8(vtbl2_u8(uint32x4_to_8x8x2(tmp0), vget_high_u8(swizzle))));
                  tmp1 = vcombine_u32(vreinterpret_u32_u8(vtbl2_u8(uint32x4_to_8x8x2(tmp1), vget_low_u8(swizzle))),
                                      vreinterpret_u32_u8(vtbl2_u8(uint32x4_to_8x8x2(tmp1), vget_high_u8(swizzle))));
#pragma GCC diagnostic pop

                  vst1q_u32(&dest[0x28], tmp0);
                  vst1q_u32(&dest[0x38], tmp1);

                  /* missaligned, so separate loads */
                  tmp0 = vld1q_lane_u32(&src3[ 6], tmp0, 0);
                  tmp0 = vld1q_lane_u32(&src3[ 7], tmp0, 1);
                  tmp0 = vld1q_lane_u32(&src3[ 8], tmp0, 2);

                  tmp1 = vld1q_lane_u32(&src3[ 9], tmp1, 0);
                  tmp1 = vld1q_lane_u32(&src3[10], tmp1, 1);
                  tmp1 = vld1q_lane_u32(&src3[11], tmp1, 2);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-braces"
                  /* GCC incorrectly warns here, fixed in GCC 5.x */
                  tmp0 = vcombine_u32(vreinterpret_u32_u8(vtbl2_u8(uint32x4_to_8x8x2(tmp0), vget_low_u8(swizzle))),
                                      vreinterpret_u32_u8(vtbl2_u8(uint32x4_to_8x8x2(tmp0), vget_high_u8(swizzle))));
                  tmp1 = vcombine_u32(vreinterpret_u32_u8(vtbl2_u8(uint32x4_to_8x8x2(tmp1), vget_low_u8(swizzle))),
                                      vreinterpret_u32_u8(vtbl2_u8(uint32x4_to_8x8x2(tmp1), vget_high_u8(swizzle))));
#pragma GCC diagnostic pop

                  vst1q_u32(&dest[0x2c], tmp0);
                  vst1q_u32(&dest[0x3c], tmp1);
#else
                  uint32_t intA1, intB1, intC1;
                  uint32_t intA2, intB2, intC2;
                  /* Mark destination cache lines as being fully written so not read */
                  store(&dest[0x00]); store(&dest[0x08]); store(&dest[0x10]); store(&dest[0x18]);

                  intA1 = src0[0]; intB1 = src0[1]; intC1 = src0[2];
                  intA2 = src0[3]; intB2 = src0[4]; intC2 = src0[5];
                  dest[0x00] = PIX0(intA1);  dest[0x01] = PIX1(intA1, intB1); dest[0x02] = PIX2(intB1, intC1);  dest[0x03] = PIX3(intC1);
                  dest[0x10] = PIX0(intA2);  dest[0x11] = PIX1(intA2, intB2); dest[0x12] = PIX2(intB2, intC2);  dest[0x13] = PIX3(intC2);
                  intA1 = src1[0]; intB1 = src1[1]; intC1 = src1[2];
                  intA2 = src1[3]; intB2 = src1[4]; intC2 = src1[5];
                  dest[0x04] = PIX0(intA1);  dest[0x05] = PIX1(intA1, intB1); dest[0x06] = PIX2(intB1, intC1);  dest[0x07] = PIX3(intC1);
                  dest[0x14] = PIX0(intA2);  dest[0x15] = PIX1(intA2, intB2); dest[0x16] = PIX2(intB2, intC2);  dest[0x17] = PIX3(intC2);
                  intA1 = src2[0]; intB1 = src2[1]; intC1 = src2[2];
                  intA2 = src2[3]; intB2 = src2[4]; intC2 = src2[5];
                  dest[0x08] = PIX0(intA1);  dest[0x09] = PIX1(intA1, intB1); dest[0x0a] = PIX2(intB1, intC1);  dest[0x0b] = PIX3(intC1);
                  dest[0x18] = PIX0(intA2);  dest[0x19] = PIX1(intA2, intB2); dest[0x1a] = PIX2(intB2, intC2);  dest[0x1b] = PIX3(intC2);
                  intA1 = src3[0]; intB1 = src3[1]; intC1 = src3[2];
                  intA2 = src3[3]; intB2 = src3[4]; intC2 = src3[5];
                  dest[0x0c] = PIX0(intA1);  dest[0x0d] = PIX1(intA1, intB1); dest[0x0e] = PIX2(intB1, intC1);  dest[0x0f] = PIX3(intC1);
                  dest[0x1c] = PIX0(intA2);  dest[0x1d] = PIX1(intA2, intB2); dest[0x1e] = PIX2(intB2, intC2);  dest[0x1f] = PIX3(intC2);

                  store(&dest[0x20]); store(&dest[0x28]); store(&dest[0x30]); store(&dest[0x38]);
                  intA1 = src0[6]; intB1 = src0[7];  intC1 = src0[8];
                  intA2 = src0[9]; intB2 = src0[10]; intC2 = src0[11];
                  dest[0x20] = PIX0(intA1);  dest[0x21] = PIX1(intA1, intB1); dest[0x22] = PIX2(intB1, intC1);  dest[0x23] = PIX3(intC1);
                  dest[0x30] = PIX0(intA2);  dest[0x31] = PIX1(intA2, intB2); dest[0x32] = PIX2(intB2, intC2);  dest[0x33] = PIX3(intC2);
                  intA1 = src1[6]; intB1 = src1[7]; intC1 = src1[8];
                  intA2 = src1[9]; intB2 = src1[10]; intC2 = src1[11];
                  dest[0x24] = PIX0(intA1);  dest[0x25] = PIX1(intA1, intB1); dest[0x26] = PIX2(intB1, intC1);  dest[0x27] = PIX3(intC1);
                  dest[0x34] = PIX0(intA2);  dest[0x35] = PIX1(intA2, intB2); dest[0x36] = PIX2(intB2, intC2);  dest[0x37] = PIX3(intC2);
                  intA1 = src2[6]; intB1 = src2[7]; intC1 = src2[8];
                  intA2 = src2[9]; intB2 = src2[10]; intC2 = src2[11];
                  dest[0x28] = PIX0(intA1);  dest[0x29] = PIX1(intA1, intB1); dest[0x2a] = PIX2(intB1, intC1);  dest[0x2b] = PIX3(intC1);
                  dest[0x38] = PIX0(intA2);  dest[0x39] = PIX1(intA2, intB2); dest[0x3a] = PIX2(intB2, intC2);  dest[0x3b] = PIX3(intC2);
                  intA1 = src3[6]; intB1 = src3[7]; intC1 = src3[8];
                  intA2 = src3[9]; intB2 = src3[10]; intC2 = src3[11];
                  dest[0x2c] = PIX0(intA1);  dest[0x2d] = PIX1(intA1, intB1); dest[0x2e] = PIX2(intB1, intC1);  dest[0x2f] = PIX3(intC1);
                  dest[0x3c] = PIX0(intA2);  dest[0x3d] = PIX1(intA2, intB2); dest[0x3e] = PIX2(intB2, intC2);  dest[0x3f] = PIX3(intC2);
#endif

                  src0 += SRC_STRIDE32; src1 += SRC_STRIDE32;
                  src2 += SRC_STRIDE32; src3 += SRC_STRIDE32;

                  dest = rowAddr1;
               }

               subY += 3; /* Doing 4-lines at a time, so skip another 3 */
            }
            else if (fast)
            {
               uint32_t *src_texel = (uint32_t *)src_texel_8;

#ifdef __ARM_NEON__
               uint32x4_t tmp0, tmp1;
               tmp0 = vdupq_n_u32(0);
               tmp1 = vdupq_n_u32(0);

               /* Prefetch next tile's source data */
               fetch(&src_texel[SRC_TILE_STRIDE32]);

               /* FIRST batch of 16 texels */
               tmp0 = vld1q_lane_u32(&src_texel[ 0], tmp0, 0);
               tmp0 = vld1q_lane_u32(&src_texel[ 1], tmp0, 1);
               tmp0 = vld1q_lane_u32(&src_texel[ 2], tmp0, 2);

               tmp1 = vld1q_lane_u32(&src_texel[ 3], tmp1, 0);
               tmp1 = vld1q_lane_u32(&src_texel[ 4], tmp1, 1);
               tmp1 = vld1q_lane_u32(&src_texel[ 5], tmp1, 2);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-braces"
                  /* GCC incorrectly warns here, fixed in GCC 5.x */
               tmp0 = vcombine_u32(vreinterpret_u32_u8(vtbl2_u8(uint32x4_to_8x8x2(tmp0), vget_low_u8(swizzle))),
                                    vreinterpret_u32_u8(vtbl2_u8(uint32x4_to_8x8x2(tmp0), vget_high_u8(swizzle))));
               tmp1 = vcombine_u32(vreinterpret_u32_u8(vtbl2_u8(uint32x4_to_8x8x2(tmp1), vget_low_u8(swizzle))),
                                    vreinterpret_u32_u8(vtbl2_u8(uint32x4_to_8x8x2(tmp1), vget_high_u8(swizzle))));
#pragma GCC diagnostic pop

               vst1q_u32(&rowAddr0[0x00], tmp0);
               vst1q_u32(&rowAddr0[0x10], tmp1);

               tmp0 = vld1q_lane_u32(&src_texel[ 6], tmp0, 0);
               tmp0 = vld1q_lane_u32(&src_texel[ 7], tmp0, 1);
               tmp0 = vld1q_lane_u32(&src_texel[ 8], tmp0, 2);

               tmp1 = vld1q_lane_u32(&src_texel[ 9], tmp1, 0);
               tmp1 = vld1q_lane_u32(&src_texel[10], tmp1, 1);
               tmp1 = vld1q_lane_u32(&src_texel[11], tmp1, 2);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-braces"
                  /* GCC incorrectly warns here, fixed in GCC 5.x */
               tmp0 = vcombine_u32(vreinterpret_u32_u8(vtbl2_u8(uint32x4_to_8x8x2(tmp0), vget_low_u8(swizzle))),
                                    vreinterpret_u32_u8(vtbl2_u8(uint32x4_to_8x8x2(tmp0), vget_high_u8(swizzle))));
               tmp1 = vcombine_u32(vreinterpret_u32_u8(vtbl2_u8(uint32x4_to_8x8x2(tmp1), vget_low_u8(swizzle))),
                                    vreinterpret_u32_u8(vtbl2_u8(uint32x4_to_8x8x2(tmp1), vget_high_u8(swizzle))));
#pragma GCC diagnostic pop

               vst1q_u32(&rowAddr0[0x20], tmp0);
               vst1q_u32(&rowAddr0[0x30], tmp1);

               /* SECOND batch of 16 texels */
               tmp0 = vld1q_lane_u32(&src_texel[12], tmp0, 0);
               tmp0 = vld1q_lane_u32(&src_texel[13], tmp0, 1);
               tmp0 = vld1q_lane_u32(&src_texel[14], tmp0, 2);

               tmp1 = vld1q_lane_u32(&src_texel[15], tmp1, 0);
               tmp1 = vld1q_lane_u32(&src_texel[16], tmp1, 1);
               tmp1 = vld1q_lane_u32(&src_texel[17], tmp1, 2);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-braces"
                  /* GCC incorrectly warns here, fixed in GCC 5.x */
               tmp0 = vcombine_u32(vreinterpret_u32_u8(vtbl2_u8(uint32x4_to_8x8x2(tmp0), vget_low_u8(swizzle))),
                                    vreinterpret_u32_u8(vtbl2_u8(uint32x4_to_8x8x2(tmp0), vget_high_u8(swizzle))));
               tmp1 = vcombine_u32(vreinterpret_u32_u8(vtbl2_u8(uint32x4_to_8x8x2(tmp1), vget_low_u8(swizzle))),
                                    vreinterpret_u32_u8(vtbl2_u8(uint32x4_to_8x8x2(tmp1), vget_high_u8(swizzle))));
#pragma GCC diagnostic pop

               vst1q_u32(&rowAddr1[0x00], tmp0);
               vst1q_u32(&rowAddr1[0x10], tmp1);

               tmp0 = vld1q_lane_u32(&src_texel[18], tmp0, 0);
               tmp0 = vld1q_lane_u32(&src_texel[19], tmp0, 1);
               tmp0 = vld1q_lane_u32(&src_texel[20], tmp0, 2);

               tmp1 = vld1q_lane_u32(&src_texel[21], tmp1, 0);
               tmp1 = vld1q_lane_u32(&src_texel[22], tmp1, 1);
               tmp1 = vld1q_lane_u32(&src_texel[23], tmp1, 2);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-braces"
                  /* GCC incorrectly warns here, fixed in GCC 5.x */
               tmp0 = vcombine_u32(vreinterpret_u32_u8(vtbl2_u8(uint32x4_to_8x8x2(tmp0), vget_low_u8(swizzle))),
                                    vreinterpret_u32_u8(vtbl2_u8(uint32x4_to_8x8x2(tmp0), vget_high_u8(swizzle))));
               tmp1 = vcombine_u32(vreinterpret_u32_u8(vtbl2_u8(uint32x4_to_8x8x2(tmp1), vget_low_u8(swizzle))),
                                    vreinterpret_u32_u8(vtbl2_u8(uint32x4_to_8x8x2(tmp1), vget_high_u8(swizzle))));
#pragma GCC diagnostic pop

               vst1q_u32(&rowAddr1[0x20], tmp0);
               vst1q_u32(&rowAddr1[0x30], tmp1);
#else
               /* Fast path for middle of image */
               uint32_t   intA1, intB1, intC1;
               uint32_t   intA2, intB2, intC2;

               /* Prefetch next tile's source data */
               fetch(&src_texel[SRC_TILE_STRIDE32]);

               /* FIRST batch of 16 texels */
               intA1 = src_texel[0]; intB1 = src_texel[1]; intC1 = src_texel[2];
               intA2 = src_texel[3]; intB2 = src_texel[4]; intC2 = src_texel[5];

               rowAddr0[0x00] = PIX0(intA1); rowAddr0[0x01] = PIX1(intA1, intB1); rowAddr0[0x02] = PIX2(intB1, intC1); rowAddr0[0x03] = PIX3(intC1);
               rowAddr0[0x10] = PIX0(intA2); rowAddr0[0x11] = PIX1(intA2, intB2); rowAddr0[0x12] = PIX2(intB2, intC2); rowAddr0[0x13] = PIX3(intC2);

               intA1 = src_texel[6]; intB1 = src_texel[7];  intC1 = src_texel[8];
               intA2 = src_texel[9]; intB2 = src_texel[10]; intC2 = src_texel[11];

               rowAddr0[0x20] = PIX0(intA1); rowAddr0[0x21] = PIX1(intA1, intB1); rowAddr0[0x22] = PIX2(intB1, intC1); rowAddr0[0x23] = PIX3(intC1);
               rowAddr0[0x30] = PIX0(intA2); rowAddr0[0x31] = PIX1(intA2, intB2); rowAddr0[0x32] = PIX2(intB2, intC2); rowAddr0[0x33] = PIX3(intC2);

               /* SECOND batch of 16 texels */
               intA1 = src_texel[12]; intB1 = src_texel[13]; intC1 = src_texel[14];
               intA2 = src_texel[15]; intB2 = src_texel[16]; intC2 = src_texel[17];

               rowAddr1[0x00] = PIX0(intA1); rowAddr1[0x01] = PIX1(intA1, intB1); rowAddr1[0x02] = PIX2(intB1, intC1); rowAddr1[0x03] = PIX3(intC1);
               rowAddr1[0x10] = PIX0(intA2); rowAddr1[0x11] = PIX1(intA2, intB2); rowAddr1[0x12] = PIX2(intB2, intC2); rowAddr1[0x13] = PIX3(intC2);

               intA1 = src_texel[18]; intB1 = src_texel[19]; intC1 = src_texel[20];
               intA2 = src_texel[21]; intB2 = src_texel[22]; intC2 = src_texel[23];

               rowAddr1[0x20] = PIX0(intA1); rowAddr1[0x21] = PIX1(intA1, intB1); rowAddr1[0x22] = PIX2(intB1, intC1); rowAddr1[0x23] = PIX3(intC1);
               rowAddr1[0x30] = PIX0(intA2); rowAddr1[0x31] = PIX1(intA2, intB2); rowAddr1[0x32] = PIX2(intB2, intC2); rowAddr1[0x33] = PIX3(intC2);
#endif
            }
            else
            {
               uint8_t  *src_texel = src_texel_8;

               /* Slower path for edges of image */
               uint32_t   endSubX1 = endSubX < (SUB_TILE_WIDTH - 1) ? endSubX : SUB_TILE_WIDTH - 1;

               for (subX = startSubX; subX <= endSubX1; ++subX)
               {
                  uint32_t r = *src_texel++;
                  uint32_t g = *src_texel++;
                  uint32_t b = *src_texel++;
                  uint32_t *dst_texel = rowAddr0 + XSWIZZLE32(subX);

                  *dst_texel = PIXEL(r, g, b);
               }

               for (subX = startSubX > SUB_TILE_WIDTH ? startSubX : SUB_TILE_WIDTH; subX <= endSubX; ++subX)
               {
                  uint32_t r = *src_texel++;
                  uint32_t g = *src_texel++;
                  uint32_t b = *src_texel++;
                  uint32_t *dst_texel = rowAddr1 + XSWIZZLE32(subX & 0xf);

                  *dst_texel = PIXEL(r, g, b);
               }
            }
         }
      }
   }

   return true;
}

#undef PIX0
#undef PIX1
#undef PIX2
#undef PIX3
#undef PIXEL

/* khrn_copy_16_to_tf16
 *
 * Copies an array in 16-bit linear format to the equivalent 16-bit Tformat array
 */
static bool OPTIMIZE_FN khrn_copy_16_to_tf16(
   uint8_t   *dst,                             /* address of Tformat image                                             */
   uint32_t  dstWidth,                         /* width of destination in texels                                       */
   uint32_t  dstStartX, uint32_t dstStartY,    /* bottom left of destination region                                    */
   uint32_t  width,     uint32_t height,       /* top right of destination region                                      */
   uint8_t   *src,                             /* address of linear image                                              */
   uint32_t  srcStartX, uint32_t srcStartY,    /* bottom left of source region                                         */
   uint32_t  srcStride,                        /* stride of source image in bytes (i.e. line length including padding) */
   int32_t   srcDir)
{
   static const uint32_t   TILE_XSHIFT       = 6;
   static const uint32_t   TILE_XBITS        = 0x3f;
   static const uint32_t   TILE_YSHIFT       = 5;
   static const uint32_t   TILE_YBITS        = 0x1f;
   static const uint32_t   SUB_TILE_WIDTH    = 32;
   static const uint32_t   TEXEL_SIZE        = 2;
   static const uint32_t   SRC_TILE_STRIDE32 = 32;
   static const uint32_t   SRC_STRIDE32      = 16;

   uint32_t   srcStride32 = srcStride / 4;
   uint32_t   blockY, blockX, subY, subX;

   uint32_t   dstBlockWidth  = ((dstWidth + TILE_XBITS) & ~TILE_XBITS) >> TILE_XSHIFT;  /* Width in 4k tiles */
   uint32_t   dstBlockStride = dstBlockWidth * TILE_SIZE;

   uint32_t   dstEndX    = dstStartX + width  - 1;
   uint32_t   dstEndY    = dstStartY + height - 1;

   uint32_t   startBlockX = dstStartX >> TILE_XSHIFT;
   uint32_t   startBlockY = dstStartY >> TILE_YSHIFT;
   uint32_t   endBlockX   = dstEndX   >> TILE_XSHIFT;
   uint32_t   endBlockY   = dstEndY   >> TILE_YSHIFT;

   int   offX = srcStartX - dstStartX;
   int   offY = srcStartY - dstStartY;

   /* Doesn't support flipped order */
   if (srcDir != 1)
      return false;

   /* For each 4k tile vertically */
   for (blockY = startBlockY; blockY <= endBlockY; ++blockY)
   {
      uint32_t   startSubY = blockY == startBlockY ? (dstStartY & TILE_YBITS) : 0;
      uint32_t   endSubY   = blockY == endBlockY   ? (dstEndY   & TILE_YBITS) : TILE_YBITS;

      uint32_t   odd = blockY & 1;
      uint32_t   mask = odd ? MAP_ODD : MAP_EVEN;

      /* For each 4k tile horizontally */
      for (blockX = startBlockX; blockX <= endBlockX; ++blockX)
      {
         uint32_t   startSubX = blockX == startBlockX ? (dstStartX & TILE_XBITS) : 0;
         uint32_t   endSubX   = blockX == endBlockX   ? (dstEndX   & TILE_XBITS) : TILE_XBITS;
         bool       fast      = (offX & 1) == 0 && startSubX == 0 && endSubX == TILE_XBITS;

         uint32_t   blockXR   = odd ? dstBlockWidth - blockX - 1 : blockX;

         /* Address of tile */
         uint8_t    *tileAddr  = dst + blockY * dstBlockStride + blockXR * TILE_SIZE;
         uint32_t   Y = (blockY << TILE_YSHIFT) + offY;
         uint32_t   X = (blockX << TILE_XSHIFT) + startSubX + offX;

         /* For each row in the tile */
         for (subY = startSubY; subY <= endSubY; ++subY)
         {
            uint8_t   *src_texel_8 = src + (Y + subY) * srcStride + X * TEXEL_SIZE;

            uint32_t offset      = mask >> ((subY & 0x10) >> 1);
            uint8_t  *rowAddr    = tileAddr + YSWIZZLE16(subY);
            uint8_t  *rowAddr0_8 = rowAddr + ((offset & 0x0f) << 8);
            uint8_t  *rowAddr1_8 = rowAddr + ((offset & 0xf0) << 4);
            bool     extra_fast  = fast && ((subY & 0x3) == 0) && ((subY + 0x3) <= endSubY);

            if (extra_fast)
            {
               /* This case copies four lines at a time, and writes a whole destination cache line at a time
                  which allows the use of "prepare for store" prefetch instructions eliminating the need to
                  load data into the cache for immediate overwrite */
               uint32_t *src_texel = (uint32_t *)src_texel_8;
               uint32_t *rowAddr0  = (uint32_t *)rowAddr0_8;
               uint32_t *rowAddr1  = (uint32_t *)rowAddr1_8;

               uint32_t *src0 = src_texel;
               uint32_t *src1 = src0 + srcStride32;
               uint32_t *src2 = src1 + srcStride32;
               uint32_t *src3 = src2 + srcStride32;
               uint32_t *dest = rowAddr0;
               uint32_t subtile;

               /* Prefetches the start of the next tile */
               fetch(&src0[SRC_TILE_STRIDE32]); fetch(&src1[SRC_TILE_STRIDE32]);
               fetch(&src2[SRC_TILE_STRIDE32]); fetch(&src3[SRC_TILE_STRIDE32]);

               for (subtile = 0; subtile < 2; ++subtile)
               {
#ifdef __ARM_NEON__
                  uint32x4_t tmp0, tmp1, tmp2, tmp3;
                  tmp0 = vdupq_n_u32(0);
                  tmp1 = vdupq_n_u32(0);
                  tmp2 = vdupq_n_u32(0);
                  tmp3 = vdupq_n_u32(0);

                  tmp0 = vld1q_lane_u32(&src0[ 0], tmp0, 0);
                  tmp0 = vld1q_lane_u32(&src0[ 1], tmp0, 1);
                  tmp0 = vld1q_lane_u32(&src0[ 2], tmp0, 2);
                  tmp0 = vld1q_lane_u32(&src0[ 3], tmp0, 3);

                  tmp1 = vld1q_lane_u32(&src1[ 0], tmp1, 0);
                  tmp1 = vld1q_lane_u32(&src1[ 1], tmp1, 1);
                  tmp1 = vld1q_lane_u32(&src1[ 2], tmp1, 2);
                  tmp1 = vld1q_lane_u32(&src1[ 3], tmp1, 3);

                  tmp2 = vld1q_lane_u32(&src2[ 0], tmp2, 0);
                  tmp2 = vld1q_lane_u32(&src2[ 1], tmp2, 1);
                  tmp2 = vld1q_lane_u32(&src2[ 2], tmp2, 2);
                  tmp2 = vld1q_lane_u32(&src2[ 3], tmp2, 3);

                  tmp3 = vld1q_lane_u32(&src3[ 0], tmp3, 0);
                  tmp3 = vld1q_lane_u32(&src3[ 1], tmp3, 1);
                  tmp3 = vld1q_lane_u32(&src3[ 2], tmp3, 2);
                  tmp3 = vld1q_lane_u32(&src3[ 3], tmp3, 3);

                  vst1q_u32(&dest[0x00], tmp0);
                  vst1q_u32(&dest[0x04], tmp1);
                  vst1q_u32(&dest[0x08], tmp2);
                  vst1q_u32(&dest[0x0c], tmp3);

                  tmp0 = vld1q_lane_u32(&src0[ 4], tmp0, 0);
                  tmp0 = vld1q_lane_u32(&src0[ 5], tmp0, 1);
                  tmp0 = vld1q_lane_u32(&src0[ 6], tmp0, 2);
                  tmp0 = vld1q_lane_u32(&src0[ 7], tmp0, 3);

                  tmp1 = vld1q_lane_u32(&src1[ 4], tmp1, 0);
                  tmp1 = vld1q_lane_u32(&src1[ 5], tmp1, 1);
                  tmp1 = vld1q_lane_u32(&src1[ 6], tmp1, 2);
                  tmp1 = vld1q_lane_u32(&src1[ 7], tmp1, 3);

                  tmp2 = vld1q_lane_u32(&src2[ 4], tmp2, 0);
                  tmp2 = vld1q_lane_u32(&src2[ 5], tmp2, 1);
                  tmp2 = vld1q_lane_u32(&src2[ 6], tmp2, 2);
                  tmp2 = vld1q_lane_u32(&src2[ 7], tmp2, 3);

                  tmp3 = vld1q_lane_u32(&src3[ 4], tmp3, 0);
                  tmp3 = vld1q_lane_u32(&src3[ 5], tmp3, 1);
                  tmp3 = vld1q_lane_u32(&src3[ 6], tmp3, 2);
                  tmp3 = vld1q_lane_u32(&src3[ 7], tmp3, 3);

                  vst1q_u32(&dest[0x10], tmp0);
                  vst1q_u32(&dest[0x14], tmp1);
                  vst1q_u32(&dest[0x18], tmp2);
                  vst1q_u32(&dest[0x1c], tmp3);

                  tmp0 = vld1q_lane_u32(&src0[ 8], tmp0, 0);
                  tmp0 = vld1q_lane_u32(&src0[ 9], tmp0, 1);
                  tmp0 = vld1q_lane_u32(&src0[10], tmp0, 2);
                  tmp0 = vld1q_lane_u32(&src0[11], tmp0, 3);

                  tmp1 = vld1q_lane_u32(&src1[ 8], tmp1, 0);
                  tmp1 = vld1q_lane_u32(&src1[ 9], tmp1, 1);
                  tmp1 = vld1q_lane_u32(&src1[10], tmp1, 2);
                  tmp1 = vld1q_lane_u32(&src1[11], tmp1, 3);

                  tmp2 = vld1q_lane_u32(&src2[ 8], tmp2, 0);
                  tmp2 = vld1q_lane_u32(&src2[ 9], tmp2, 1);
                  tmp2 = vld1q_lane_u32(&src2[10], tmp2, 2);
                  tmp2 = vld1q_lane_u32(&src2[11], tmp2, 3);

                  tmp3 = vld1q_lane_u32(&src3[ 8], tmp3, 0);
                  tmp3 = vld1q_lane_u32(&src3[ 9], tmp3, 1);
                  tmp3 = vld1q_lane_u32(&src3[10], tmp3, 2);
                  tmp3 = vld1q_lane_u32(&src3[11], tmp3, 3);

                  vst1q_u32(&dest[0x20], tmp0);
                  vst1q_u32(&dest[0x24], tmp1);
                  vst1q_u32(&dest[0x28], tmp2);
                  vst1q_u32(&dest[0x2c], tmp3);

                  tmp0 = vld1q_lane_u32(&src0[12], tmp0, 0);
                  tmp0 = vld1q_lane_u32(&src0[13], tmp0, 1);
                  tmp0 = vld1q_lane_u32(&src0[14], tmp0, 2);
                  tmp0 = vld1q_lane_u32(&src0[15], tmp0, 3);

                  tmp1 = vld1q_lane_u32(&src1[12], tmp1, 0);
                  tmp1 = vld1q_lane_u32(&src1[13], tmp1, 1);
                  tmp1 = vld1q_lane_u32(&src1[14], tmp1, 2);
                  tmp1 = vld1q_lane_u32(&src1[15], tmp1, 3);

                  tmp2 = vld1q_lane_u32(&src2[12], tmp2, 0);
                  tmp2 = vld1q_lane_u32(&src2[13], tmp2, 1);
                  tmp2 = vld1q_lane_u32(&src2[14], tmp2, 2);
                  tmp2 = vld1q_lane_u32(&src2[15], tmp2, 3);

                  tmp3 = vld1q_lane_u32(&src3[12], tmp3, 0);
                  tmp3 = vld1q_lane_u32(&src3[13], tmp3, 1);
                  tmp3 = vld1q_lane_u32(&src3[14], tmp3, 2);
                  tmp3 = vld1q_lane_u32(&src3[15], tmp3, 3);

                  vst1q_u32(&dest[0x30], tmp0);
                  vst1q_u32(&dest[0x34], tmp1);
                  vst1q_u32(&dest[0x38], tmp2);
                  vst1q_u32(&dest[0x3c], tmp3);
#else
                  /* Mark destination cache lines as being fully written so not read */
                  store(&dest[0x00]); store(&dest[0x08]); store(&dest[0x10]); store(&dest[0x18]);
                  dest[0x00] = src0[0];   dest[0x01] = src0[1];   dest[0x02] = src0[2];   dest[0x03] = src0[3];
                  dest[0x04] = src1[0];   dest[0x05] = src1[1];   dest[0x06] = src1[2];   dest[0x07] = src1[3];
                  dest[0x08] = src2[0];   dest[0x09] = src2[1];   dest[0x0a] = src2[2];   dest[0x0b] = src2[3];
                  dest[0x0c] = src3[0];   dest[0x0d] = src3[1];   dest[0x0e] = src3[2];   dest[0x0f] = src3[3];
                  dest[0x10] = src0[4];   dest[0x11] = src0[5];   dest[0x12] = src0[6];   dest[0x13] = src0[7];
                  dest[0x14] = src1[4];   dest[0x15] = src1[5];   dest[0x16] = src1[6];   dest[0x17] = src1[7];
                  dest[0x18] = src2[4];   dest[0x19] = src2[5];   dest[0x1a] = src2[6];   dest[0x1b] = src2[7];
                  dest[0x1c] = src3[4];   dest[0x1d] = src3[5];   dest[0x1e] = src3[6];   dest[0x1f] = src3[7];

                  store(&dest[0x20]); store(&dest[0x28]); store(&dest[0x30]); store(&dest[0x38]);
                  dest[0x20] = src0[8];   dest[0x21] = src0[9];   dest[0x22] = src0[10];  dest[0x23] = src0[11];
                  dest[0x24] = src1[8];   dest[0x25] = src1[9];   dest[0x26] = src1[10];  dest[0x27] = src1[11];
                  dest[0x28] = src2[8];   dest[0x29] = src2[9];   dest[0x2a] = src2[10];  dest[0x2b] = src2[11];
                  dest[0x2c] = src3[8];   dest[0x2d] = src3[9];   dest[0x2e] = src3[10];  dest[0x2f] = src3[11];
                  dest[0x30] = src0[12];  dest[0x31] = src0[13];  dest[0x32] = src0[14];  dest[0x33] = src0[15];
                  dest[0x34] = src1[12];  dest[0x35] = src1[13];  dest[0x36] = src1[14];  dest[0x37] = src1[15];
                  dest[0x38] = src2[12];  dest[0x39] = src2[13];  dest[0x3a] = src2[14];  dest[0x3b] = src2[15];
                  dest[0x3c] = src3[12];  dest[0x3d] = src3[13];  dest[0x3e] = src3[14];  dest[0x3f] = src3[15];
#endif

                  src0 += SRC_STRIDE32; src1 += SRC_STRIDE32;
                  src2 += SRC_STRIDE32; src3 += SRC_STRIDE32;

                  dest = rowAddr1;
               }

               subY += 3; /* Doing 4-lines at a time, so skip another 3 */
            }
            else if (fast)
            {
               uint32_t *src_texel = (uint32_t *)src_texel_8;
               uint32_t *rowAddr0  = (uint32_t *)rowAddr0_8;
               uint32_t *rowAddr1  = (uint32_t *)rowAddr1_8;

#ifdef __ARM_NEON__
               uint32x4_t tmp0, tmp1, tmp2, tmp3;

               fetch(&src_texel[SRC_TILE_STRIDE32]);

               tmp0 = vdupq_n_u32(0);
               tmp1 = vdupq_n_u32(0);
               tmp2 = vdupq_n_u32(0);
               tmp3 = vdupq_n_u32(0);

               tmp0 = vld1q_lane_u32(&src_texel[ 0], tmp0, 0);
               tmp0 = vld1q_lane_u32(&src_texel[ 1], tmp0, 1);
               tmp0 = vld1q_lane_u32(&src_texel[ 2], tmp0, 2);
               tmp0 = vld1q_lane_u32(&src_texel[ 3], tmp0, 3);

               tmp1 = vld1q_lane_u32(&src_texel[ 4], tmp1, 0);
               tmp1 = vld1q_lane_u32(&src_texel[ 5], tmp1, 1);
               tmp1 = vld1q_lane_u32(&src_texel[ 6], tmp1, 2);
               tmp1 = vld1q_lane_u32(&src_texel[ 7], tmp1, 3);

               tmp2 = vld1q_lane_u32(&src_texel[ 8], tmp2, 0);
               tmp2 = vld1q_lane_u32(&src_texel[ 9], tmp2, 1);
               tmp2 = vld1q_lane_u32(&src_texel[10], tmp2, 2);
               tmp2 = vld1q_lane_u32(&src_texel[11], tmp2, 3);

               tmp3 = vld1q_lane_u32(&src_texel[12], tmp3, 0);
               tmp3 = vld1q_lane_u32(&src_texel[13], tmp3, 1);
               tmp3 = vld1q_lane_u32(&src_texel[14], tmp3, 2);
               tmp3 = vld1q_lane_u32(&src_texel[15], tmp3, 3);

               vst1q_u32(&rowAddr0[0x00], tmp0);
               vst1q_u32(&rowAddr0[0x10], tmp1);
               vst1q_u32(&rowAddr0[0x20], tmp2);
               vst1q_u32(&rowAddr0[0x30], tmp3);

               tmp0 = vld1q_lane_u32(&src_texel[16], tmp0, 0);
               tmp0 = vld1q_lane_u32(&src_texel[17], tmp0, 1);
               tmp0 = vld1q_lane_u32(&src_texel[18], tmp0, 2);
               tmp0 = vld1q_lane_u32(&src_texel[19], tmp0, 3);

               tmp1 = vld1q_lane_u32(&src_texel[20], tmp1, 0);
               tmp1 = vld1q_lane_u32(&src_texel[21], tmp1, 1);
               tmp1 = vld1q_lane_u32(&src_texel[22], tmp1, 2);
               tmp1 = vld1q_lane_u32(&src_texel[23], tmp1, 3);

               tmp2 = vld1q_lane_u32(&src_texel[24], tmp2, 0);
               tmp2 = vld1q_lane_u32(&src_texel[25], tmp2, 1);
               tmp2 = vld1q_lane_u32(&src_texel[26], tmp2, 2);
               tmp2 = vld1q_lane_u32(&src_texel[27], tmp2, 3);

               tmp3 = vld1q_lane_u32(&src_texel[28], tmp3, 0);
               tmp3 = vld1q_lane_u32(&src_texel[29], tmp3, 1);
               tmp3 = vld1q_lane_u32(&src_texel[30], tmp3, 2);
               tmp3 = vld1q_lane_u32(&src_texel[31], tmp3, 3);

               vst1q_u32(&rowAddr1[0x00], tmp0);
               vst1q_u32(&rowAddr1[0x10], tmp1);
               vst1q_u32(&rowAddr1[0x20], tmp2);
               vst1q_u32(&rowAddr1[0x30], tmp3);
#else
               fetch(&src_texel[SRC_TILE_STRIDE32]);

               /* Fast path for centre of image */
               /* Copy as integers for speed    */
               rowAddr0[0x00] = src_texel[0];   rowAddr0[0x01] = src_texel[1];
               rowAddr0[0x02] = src_texel[2];   rowAddr0[0x03] = src_texel[3];
               rowAddr0[0x10] = src_texel[4];   rowAddr0[0x11] = src_texel[5];
               rowAddr0[0x12] = src_texel[6];   rowAddr0[0x13] = src_texel[7];
               rowAddr0[0x20] = src_texel[8];   rowAddr0[0x21] = src_texel[9];
               rowAddr0[0x22] = src_texel[10];  rowAddr0[0x23] = src_texel[11];
               rowAddr0[0x30] = src_texel[12];  rowAddr0[0x31] = src_texel[13];
               rowAddr0[0x32] = src_texel[14];  rowAddr0[0x33] = src_texel[15];

               rowAddr1[0x00] = src_texel[16];  rowAddr1[0x01] = src_texel[17];
               rowAddr1[0x02] = src_texel[18];  rowAddr1[0x03] = src_texel[19];
               rowAddr1[0x10] = src_texel[20];  rowAddr1[0x11] = src_texel[21];
               rowAddr1[0x12] = src_texel[22];  rowAddr1[0x13] = src_texel[23];
               rowAddr1[0x20] = src_texel[24];  rowAddr1[0x21] = src_texel[25];
               rowAddr1[0x22] = src_texel[26];  rowAddr1[0x23] = src_texel[27];
               rowAddr1[0x30] = src_texel[28];  rowAddr1[0x31] = src_texel[29];
               rowAddr1[0x32] = src_texel[30];  rowAddr1[0x33] = src_texel[31];
#endif
            }
            else
            {
               uint16_t *src_texel = (uint16_t *)src_texel_8;
               uint16_t *rowAddr0  = (uint16_t *)rowAddr0_8;
               uint16_t *rowAddr1  = (uint16_t *)rowAddr1_8;

               /* Slower path for edges of image */
               uint32_t   endSubX1 = endSubX < (SUB_TILE_WIDTH - 1) ? endSubX : SUB_TILE_WIDTH - 1;

               for (subX = startSubX; subX <= endSubX1; ++subX)
               {
                  uint16_t *dst_texel =  rowAddr0 + XSWIZZLE16(subX);
                  *dst_texel = *src_texel;
                  src_texel += 1;
               }

               for (subX = startSubX > SUB_TILE_WIDTH ? startSubX : SUB_TILE_WIDTH; subX <= endSubX; ++subX)
               {
                  uint16_t *dst_texel =  rowAddr1 + XSWIZZLE16(subX);
                  *dst_texel = *src_texel;
                  src_texel += 1;
               }
            }
         }
      }
   }

   return true;
}

/* khrn_copy_8_to_tf8
 *
 * Copies an array in alpha linear format to the equivalent 8-bit Tformat array
 */
static bool OPTIMIZE_FN khrn_copy_8_to_tf8(
   uint8_t   *dst,                             /* address of Tformat image                                             */
   uint32_t  dstWidth,                         /* width of destination in texels                                       */
   uint32_t  dstStartX, uint32_t dstStartY,    /* bottom left of destination region                                    */
   uint32_t  width,     uint32_t height,       /* top right of destination region                                      */
   uint8_t   *src,                             /* address of linear image                                              */
   uint32_t  srcStartX, uint32_t srcStartY,    /* bottom left of source region                                         */
   uint32_t  srcStride,                        /* stride of source image in bytes (i.e. line length including padding) */
   int32_t   srcDir)
{
   static const uint32_t   TILE_XSHIFT       = 6;
   static const uint32_t   TILE_XBITS        = 0x3f;
   static const uint32_t   TILE_YSHIFT       = 6;
   static const uint32_t   TILE_YBITS        = 0x3f;
   static const uint32_t   SUB_TILE_WIDTH    = 32;
   static const uint32_t   TEXEL_SIZE        = 1;
   static const uint32_t   SRC_TILE_STRIDE32 = 16;
   static const uint32_t   SRC_STRIDE32      = 8;

   uint32_t   srcStride32 = srcStride / 4;
   uint32_t   blockY, blockX, subY, subX;

   uint32_t   dstBlockWidth  = ((dstWidth + TILE_XBITS) & ~TILE_XBITS) >> TILE_XSHIFT;  /* Width in 4k tiles */
   uint32_t   dstBlockStride = dstBlockWidth * TILE_SIZE;

   uint32_t   dstEndX    = dstStartX + width  - 1;
   uint32_t   dstEndY    = dstStartY + height - 1;

   uint32_t   startBlockX = dstStartX >> TILE_XSHIFT;
   uint32_t   startBlockY = dstStartY >> TILE_YSHIFT;
   uint32_t   endBlockX   = dstEndX   >> TILE_XSHIFT;
   uint32_t   endBlockY   = dstEndY   >> TILE_YSHIFT;

   int   offX = srcStartX - dstStartX;
   int   offY = srcStartY - dstStartY;

   /* Doesn't support flipped order */
   if (srcDir != 1)
      return false;

   /* For each 4k tile vertically */
   for (blockY = startBlockY; blockY <= endBlockY; ++blockY)
   {
      uint32_t   startSubY = blockY == startBlockY ? (dstStartY & TILE_YBITS) : 0;
      uint32_t   endSubY   = blockY == endBlockY   ? (dstEndY   & TILE_YBITS) : TILE_YBITS;

      uint32_t   odd = blockY & 1;
      uint32_t   mask = odd ? MAP_ODD : MAP_EVEN;

      /* For each 4k tile horizontally */
      for (blockX = startBlockX; blockX <= endBlockX; ++blockX)
      {
         uint32_t   startSubX = blockX == startBlockX ? (dstStartX & TILE_XBITS) : 0;
         uint32_t   endSubX   = blockX == endBlockX   ? (dstEndX   & TILE_XBITS) : TILE_XBITS;
         bool       fast      = (offX & 0x3) == 0 && startSubX == 0 && endSubX == TILE_XBITS && (srcStride & 0x3) == 0;

         uint32_t   blockXR   = odd ? dstBlockWidth - blockX - 1 : blockX;

         /* Address of tile */
         uint8_t  *tileAddr  = dst + blockY * dstBlockStride + blockXR * TILE_SIZE;
         uint32_t   Y = (blockY << TILE_YSHIFT) + offY;
         uint32_t   X = (blockX << TILE_XSHIFT) + startSubX + offX;

         /* For each row in the tile */
         for (subY = startSubY; subY <= endSubY; ++subY)
         {
            uint8_t  *src_texel_8 = src + (Y + subY) * srcStride + X * TEXEL_SIZE;

            uint32_t offset      = mask >> ((subY & 0x20) >> 2);
            uint8_t  *rowAddr    = tileAddr + YSWIZZLE8(subY);
            uint8_t  *rowAddr0_8 = rowAddr + ((offset & 0x0f) << 8);
            uint8_t  *rowAddr1_8 = rowAddr + ((offset & 0xf0) << 4);
            bool     extra_fast  = fast && ((subY & 0x7) == 0) && ((subY + 0x7) <= endSubY);

            if (extra_fast)
            {
               /* This case copies four lines at a time, and writes a whole destination cache line at a time
                  which allows the use of "prepare for store" prefetch instructions eliminating the need to
                  load data into the cache for immediate overwrite */
               uint32_t *src_texel = (uint32_t *)src_texel_8;
               uint32_t *rowAddr0  = (uint32_t *)rowAddr0_8;
               uint32_t *rowAddr1  = (uint32_t *)rowAddr1_8;

               uint32_t *src0 = src_texel;
               uint32_t *src1 = src0 + srcStride32;
               uint32_t *src2 = src1 + srcStride32;
               uint32_t *src3 = src2 + srcStride32;
               uint32_t *src4 = src3 + srcStride32;
               uint32_t *src5 = src4 + srcStride32;
               uint32_t *src6 = src5 + srcStride32;
               uint32_t *src7 = src6 + srcStride32;
               uint32_t *dest = rowAddr0;
               uint32_t subtile;

               /* Prefetches the start of the next tile */
               fetch(&src0[SRC_TILE_STRIDE32]); fetch(&src1[SRC_TILE_STRIDE32]);
               fetch(&src2[SRC_TILE_STRIDE32]); fetch(&src3[SRC_TILE_STRIDE32]);
               fetch(&src4[SRC_TILE_STRIDE32]); fetch(&src5[SRC_TILE_STRIDE32]);
               fetch(&src6[SRC_TILE_STRIDE32]); fetch(&src7[SRC_TILE_STRIDE32]);

               for (subtile = 0; subtile < 2; ++subtile)
               {
#ifdef __ARM_NEON__
                  uint32x4_t tmp0, tmp1, tmp2, tmp3;

                  tmp0 = vdupq_n_u32(0);
                  tmp1 = vdupq_n_u32(0);
                  tmp2 = vdupq_n_u32(0);
                  tmp3 = vdupq_n_u32(0);

                  tmp0 = vld1q_lane_u32(&src0[ 0], tmp0, 0);
                  tmp0 = vld1q_lane_u32(&src0[ 1], tmp0, 1);
                  tmp0 = vld1q_lane_u32(&src1[ 0], tmp0, 2);
                  tmp0 = vld1q_lane_u32(&src1[ 1], tmp0, 3);

                  tmp1 = vld1q_lane_u32(&src2[ 0], tmp1, 0);
                  tmp1 = vld1q_lane_u32(&src2[ 1], tmp1, 1);
                  tmp1 = vld1q_lane_u32(&src3[ 0], tmp1, 2);
                  tmp1 = vld1q_lane_u32(&src3[ 1], tmp1, 3);

                  tmp2 = vld1q_lane_u32(&src4[ 0], tmp2, 0);
                  tmp2 = vld1q_lane_u32(&src4[ 1], tmp2, 1);
                  tmp2 = vld1q_lane_u32(&src5[ 0], tmp2, 2);
                  tmp2 = vld1q_lane_u32(&src5[ 1], tmp2, 3);

                  tmp3 = vld1q_lane_u32(&src6[ 0], tmp3, 0);
                  tmp3 = vld1q_lane_u32(&src6[ 1], tmp3, 1);
                  tmp3 = vld1q_lane_u32(&src7[ 0], tmp3, 2);
                  tmp3 = vld1q_lane_u32(&src7[ 1], tmp3, 3);

                  vst1q_u32(&dest[0x00], tmp0);
                  vst1q_u32(&dest[0x04], tmp1);
                  vst1q_u32(&dest[0x08], tmp2);
                  vst1q_u32(&dest[0x0c], tmp3);

                  tmp0 = vld1q_lane_u32(&src0[ 2], tmp0, 0);
                  tmp0 = vld1q_lane_u32(&src0[ 3], tmp0, 1);
                  tmp0 = vld1q_lane_u32(&src1[ 2], tmp0, 2);
                  tmp0 = vld1q_lane_u32(&src1[ 3], tmp0, 3);

                  tmp1 = vld1q_lane_u32(&src2[ 2], tmp1, 0);
                  tmp1 = vld1q_lane_u32(&src2[ 3], tmp1, 1);
                  tmp1 = vld1q_lane_u32(&src3[ 2], tmp1, 2);
                  tmp1 = vld1q_lane_u32(&src3[ 3], tmp1, 3);

                  tmp2 = vld1q_lane_u32(&src4[ 2], tmp2, 0);
                  tmp2 = vld1q_lane_u32(&src4[ 3], tmp2, 1);
                  tmp2 = vld1q_lane_u32(&src5[ 2], tmp2, 2);
                  tmp2 = vld1q_lane_u32(&src5[ 3], tmp2, 3);

                  tmp3 = vld1q_lane_u32(&src6[ 2], tmp3, 0);
                  tmp3 = vld1q_lane_u32(&src6[ 3], tmp3, 1);
                  tmp3 = vld1q_lane_u32(&src7[ 2], tmp3, 2);
                  tmp3 = vld1q_lane_u32(&src7[ 3], tmp3, 3);

                  vst1q_u32(&dest[0x10], tmp0);
                  vst1q_u32(&dest[0x14], tmp1);
                  vst1q_u32(&dest[0x18], tmp2);
                  vst1q_u32(&dest[0x1c], tmp3);

                  tmp0 = vld1q_lane_u32(&src0[ 4], tmp0, 0);
                  tmp0 = vld1q_lane_u32(&src0[ 5], tmp0, 1);
                  tmp0 = vld1q_lane_u32(&src1[ 4], tmp0, 2);
                  tmp0 = vld1q_lane_u32(&src1[ 5], tmp0, 3);

                  tmp1 = vld1q_lane_u32(&src2[ 4], tmp1, 0);
                  tmp1 = vld1q_lane_u32(&src2[ 5], tmp1, 1);
                  tmp1 = vld1q_lane_u32(&src3[ 4], tmp1, 2);
                  tmp1 = vld1q_lane_u32(&src3[ 5], tmp1, 3);

                  tmp2 = vld1q_lane_u32(&src4[ 4], tmp2, 0);
                  tmp2 = vld1q_lane_u32(&src4[ 5], tmp2, 1);
                  tmp2 = vld1q_lane_u32(&src5[ 4], tmp2, 2);
                  tmp2 = vld1q_lane_u32(&src5[ 5], tmp2, 3);

                  tmp3 = vld1q_lane_u32(&src6[ 4], tmp3, 0);
                  tmp3 = vld1q_lane_u32(&src6[ 5], tmp3, 1);
                  tmp3 = vld1q_lane_u32(&src7[ 4], tmp3, 2);
                  tmp3 = vld1q_lane_u32(&src7[ 5], tmp3, 3);

                  vst1q_u32(&dest[0x20], tmp0);
                  vst1q_u32(&dest[0x24], tmp1);
                  vst1q_u32(&dest[0x28], tmp2);
                  vst1q_u32(&dest[0x2c], tmp3);

                  tmp0 = vld1q_lane_u32(&src0[ 6], tmp0, 0);
                  tmp0 = vld1q_lane_u32(&src0[ 7], tmp0, 1);
                  tmp0 = vld1q_lane_u32(&src1[ 6], tmp0, 2);
                  tmp0 = vld1q_lane_u32(&src1[ 7], tmp0, 3);

                  tmp1 = vld1q_lane_u32(&src2[ 6], tmp1, 0);
                  tmp1 = vld1q_lane_u32(&src2[ 7], tmp1, 1);
                  tmp1 = vld1q_lane_u32(&src3[ 6], tmp1, 2);
                  tmp1 = vld1q_lane_u32(&src3[ 7], tmp1, 3);

                  tmp2 = vld1q_lane_u32(&src4[ 6], tmp2, 0);
                  tmp2 = vld1q_lane_u32(&src4[ 7], tmp2, 1);
                  tmp2 = vld1q_lane_u32(&src5[ 6], tmp2, 2);
                  tmp2 = vld1q_lane_u32(&src5[ 7], tmp2, 3);

                  tmp3 = vld1q_lane_u32(&src6[ 6], tmp3, 0);
                  tmp3 = vld1q_lane_u32(&src6[ 7], tmp3, 1);
                  tmp3 = vld1q_lane_u32(&src7[ 6], tmp3, 2);
                  tmp3 = vld1q_lane_u32(&src7[ 7], tmp3, 3);

                  vst1q_u32(&dest[0x30], tmp0);
                  vst1q_u32(&dest[0x34], tmp1);
                  vst1q_u32(&dest[0x38], tmp2);
                  vst1q_u32(&dest[0x3c], tmp3);
#else
                  /* Mark destination cache lines as being fully written so not read */
                  store(&dest[0x00]); store(&dest[0x08]); store(&dest[0x10]); store(&dest[0x18]);
                  dest[0x00] = src0[0];  dest[0x01] = src0[1];  dest[0x02] = src1[0];  dest[0x03] = src1[1];
                  dest[0x04] = src2[0];  dest[0x05] = src2[1];  dest[0x06] = src3[0];  dest[0x07] = src3[1];
                  dest[0x08] = src4[0];  dest[0x09] = src4[1];  dest[0x0a] = src5[0];  dest[0x0b] = src5[1];
                  dest[0x0c] = src6[0];  dest[0x0d] = src6[1];  dest[0x0e] = src7[0];  dest[0x0f] = src7[1];

                  dest[0x10] = src0[2];  dest[0x11] = src0[3];  dest[0x12] = src1[2];  dest[0x13] = src1[3];
                  dest[0x14] = src2[2];  dest[0x15] = src2[3];  dest[0x16] = src3[2];  dest[0x17] = src3[3];
                  dest[0x18] = src4[2];  dest[0x19] = src4[3];  dest[0x1a] = src5[2];  dest[0x1b] = src5[3];
                  dest[0x1c] = src6[2];  dest[0x1d] = src6[3];  dest[0x1e] = src7[2];  dest[0x1f] = src7[3];

                  store(&dest[0x20]); store(&dest[0x28]); store(&dest[0x30]); store(&dest[0x38]);
                  dest[0x20] = src0[4];  dest[0x21] = src0[5];  dest[0x22] = src1[4];  dest[0x23] = src1[5];
                  dest[0x24] = src2[4];  dest[0x25] = src2[5];  dest[0x26] = src3[4];  dest[0x27] = src3[5];
                  dest[0x28] = src4[4];  dest[0x29] = src4[5];  dest[0x2a] = src5[4];  dest[0x2b] = src5[5];
                  dest[0x2c] = src6[4];  dest[0x2d] = src6[5];  dest[0x2e] = src7[4];  dest[0x2f] = src7[5];

                  dest[0x30] = src0[6];  dest[0x31] = src0[7];  dest[0x32] = src1[6];  dest[0x33] = src1[7];
                  dest[0x34] = src2[6];  dest[0x35] = src2[7];  dest[0x36] = src3[6];  dest[0x37] = src3[7];
                  dest[0x38] = src4[6];  dest[0x39] = src4[7];  dest[0x3a] = src5[6];  dest[0x3b] = src5[7];
                  dest[0x3c] = src6[6];  dest[0x3d] = src6[7];  dest[0x3e] = src7[6];  dest[0x3f] = src7[7];
#endif

                  src0 += SRC_STRIDE32; src1 += SRC_STRIDE32;
                  src2 += SRC_STRIDE32; src3 += SRC_STRIDE32;
                  src4 += SRC_STRIDE32; src5 += SRC_STRIDE32;
                  src6 += SRC_STRIDE32; src7 += SRC_STRIDE32;

                  dest = rowAddr1;
               }

               subY += 7; /* Doing 8-lines at a time, so skip another 7 */
            }
            else if (fast)
            {
               uint32_t *src_texel = (uint32_t *)src_texel_8;
               uint32_t *rowAddr0  = (uint32_t *)rowAddr0_8;
               uint32_t *rowAddr1  = (uint32_t *)rowAddr1_8;

#ifdef __ARM_NEON__
               uint32x2_t tmp0, tmp1, tmp2, tmp3;

               /* Prefetch next tiles data */
               fetch(&src_texel[SRC_TILE_STRIDE32]);

               tmp0 = vdup_n_u32(0);
               tmp1 = vdup_n_u32(0);
               tmp2 = vdup_n_u32(0);
               tmp3 = vdup_n_u32(0);

               tmp0 = vld1_lane_u32(&src_texel[0], tmp0, 0);
               tmp0 = vld1_lane_u32(&src_texel[1], tmp0, 1);
               tmp1 = vld1_lane_u32(&src_texel[2], tmp1, 0);
               tmp1 = vld1_lane_u32(&src_texel[3], tmp1, 1);
               tmp2 = vld1_lane_u32(&src_texel[4], tmp2, 0);
               tmp2 = vld1_lane_u32(&src_texel[5], tmp2, 1);
               tmp3 = vld1_lane_u32(&src_texel[6], tmp3, 0);
               tmp3 = vld1_lane_u32(&src_texel[7], tmp3, 1);

               vst1_u32(&rowAddr0[0x00], tmp0);
               vst1_u32(&rowAddr0[0x10], tmp1);
               vst1_u32(&rowAddr0[0x20], tmp2);
               vst1_u32(&rowAddr0[0x30], tmp3);

               tmp0 = vld1_lane_u32(&src_texel[ 8], tmp0, 0);
               tmp0 = vld1_lane_u32(&src_texel[ 9], tmp0, 1);
               tmp1 = vld1_lane_u32(&src_texel[10], tmp1, 0);
               tmp1 = vld1_lane_u32(&src_texel[11], tmp1, 1);
               tmp2 = vld1_lane_u32(&src_texel[12], tmp2, 0);
               tmp2 = vld1_lane_u32(&src_texel[13], tmp2, 1);
               tmp3 = vld1_lane_u32(&src_texel[14], tmp3, 0);
               tmp3 = vld1_lane_u32(&src_texel[15], tmp3, 1);

               vst1_u32(&rowAddr1[0x00], tmp0);
               vst1_u32(&rowAddr1[0x10], tmp1);
               vst1_u32(&rowAddr1[0x20], tmp2);
               vst1_u32(&rowAddr1[0x30], tmp3);
#else
               /* Prefetch next tiles data */
               fetch(&src_texel[SRC_TILE_STRIDE32]);

               /* Fast path for centre of image */
               /* Copy as integers for speed    */
               rowAddr0[0x00] = src_texel[0];  rowAddr0[0x01] = src_texel[1];
               rowAddr0[0x10] = src_texel[2];  rowAddr0[0x11] = src_texel[3];
               rowAddr0[0x20] = src_texel[4];  rowAddr0[0x21] = src_texel[5];
               rowAddr0[0x30] = src_texel[6];  rowAddr0[0x31] = src_texel[7];

               rowAddr1[0x00] = src_texel[8];  rowAddr1[0x01] = src_texel[9];
               rowAddr1[0x10] = src_texel[10]; rowAddr1[0x11] = src_texel[11];
               rowAddr1[0x20] = src_texel[12]; rowAddr1[0x21] = src_texel[13];
               rowAddr1[0x30] = src_texel[14]; rowAddr1[0x31] = src_texel[15];
#endif
            }
            else
            {
               uint8_t *src_texel = src_texel_8;
               uint8_t *rowAddr0  = rowAddr0_8;
               uint8_t *rowAddr1  = rowAddr1_8;

               /* Slower path for edges of image */
               uint32_t   endSubX1 = endSubX < (SUB_TILE_WIDTH - 1) ? endSubX : SUB_TILE_WIDTH - 1;

               for (subX = startSubX; subX <= endSubX1; ++subX)
               {
                  uint8_t *dst_texel =  rowAddr0 + XSWIZZLE8(subX);
                  *dst_texel = *src_texel;
                  src_texel += 1;
               }

               for (subX = startSubX > SUB_TILE_WIDTH ? startSubX : SUB_TILE_WIDTH; subX <= endSubX; ++subX)
               {
                  uint8_t *dst_texel =  rowAddr1 + XSWIZZLE8(subX);
                  *dst_texel = *src_texel;
                  src_texel += 1;
               }
            }
         }
      }
   }

   return true;
}

#undef XSWIZZLE8
#undef XSWIZZLE16
#undef XSWIZZLE32
#undef YSWIZZLE8
#undef YSWIZZLE16
#undef YSWIZZLE32

static bool is_stride_valid(uint32_t format, uint32_t stride)
{
   if (format == A_8 || format == L_8)
      return true;

   return (stride & 0x3) == 0;
}

typedef struct
{
   KHRN_IMAGE_WRAP_T       *dst;
   uint32_t                dst_x;
   uint32_t                dst_y;
   uint32_t                width;
   uint32_t                height;
   const KHRN_IMAGE_WRAP_T *src;
   uint32_t                src_x;
   uint32_t                src_y;
   KHRN_IMAGE_CONV_T       conv;
   bool                    retCode;
} KHRN_FAST_COPY_TO_TF_ARGS_T;

typedef struct
{
   KHRN_IMAGE_WRAP_T       *dst;
   const KHRN_IMAGE_WRAP_T *src;
   bool                    retCode;
} KHRN_WRAP_SUBSAMPLE_FAST_ARGS_T;

static bool khrn_fast_copy_to_tf(
   KHRN_IMAGE_WRAP_T *dst,
   uint32_t  dst_x, uint32_t dst_y,
   uint32_t  width,     uint32_t height,
   const KHRN_IMAGE_WRAP_T *src,
   uint32_t  src_x, uint32_t src_y,
   KHRN_IMAGE_CONV_T conv)
{
   bool     done        = false;
   int32_t  srcDir      = 1;
   uint8_t  *srcStorage = src->storage;

   if (conv != IMAGE_CONV_GL)
      return false;

   /* Special case whole buffer with no format or colour-space conversion */
   /* All sizes and strides must match, buffers must be tightly packed    */
   /* Should be used e.g. when doing CopyTexImage2D() from an FBO         */
   if (khrn_image_is_tformat(src->format) &&
       src->format == dst->format &&
       dst_x == 0 && dst_y == 0 &&
       src_x == 0 && src_y == 0 &&
       width == src->width && height == src->height &&
       width == dst->width && height == dst->height &&
       src->stride == dst->stride &&
       dst->stride == (int32_t)khrn_image_get_stride(dst->format, width))
   {
      khrn_memcpy(dst->storage, src->storage, khrn_image_get_size(dst->format, width, height));
      return true;
   }

   if ((src->flags & IMAGE_FLAG_DISPLAY) != 0)
   {
      srcStorage = (uint8_t*)src->storage + (src->height - 1) * src->stride;
      srcDir     = -1;
   }

#ifdef BIG_ENDIAN_CPU
#define DST_FORMAT RGBX_8888_TF
#else
#define DST_FORMAT XBGR_8888_TF
#endif

   /* Check for alignment */
   if (((uint32_t)src->storage & 0x3) == 0 && is_stride_valid(src->format, src->stride))
   {
      if (src->format == YUV_422)
      {
         if (dst->format == DST_FORMAT)
         {
            done = khrn_copy_YUYV_to_tf32(dst->storage, dst->width,
               dst_x, dst_y, width, height,
               src->storage, src_x, src_y,
               src->stride, srcDir);
         }
         else if (dst->format == YUV_422_RSO)
         {
            done = khrn_copy_YUYV_to_rso(dst->storage, dst->width,
               dst_x, dst_y, width, height,
               src->storage, src_x, src_y,
               src->stride, srcDir);
         }
      }
      else if (src->format == YUV_422_REV)
      {
         if (dst->format == DST_FORMAT)
         {
            done = khrn_copy_YUYV_REV_to_tf32(dst->storage, dst->width,
               dst_x, dst_y, width, height,
               src->storage, src_x, src_y,
               src->stride, srcDir);
         }
         else if (dst->format == YUV_422_RSO)
         {
            done = khrn_copy_YUYV_to_rso(dst->storage, dst->width,
               dst_x, dst_y, width, height,
               src->storage, src_x, src_y,
               src->stride, srcDir);
         }
      }
      else if (src->format == BGR_888 && dst->format == XBGR_8888_TF)
      {
         done = khrn_copy_888_to_tf32(dst->storage, dst->width,
            dst_x, dst_y, width, height,
            src->storage, src_x, src_y,
            src->stride, srcDir);
      }
      else if (khrn_image_is_rso(src->format) && khrn_image_is_tformat(dst->format) &&
         (khrn_image_to_tf_format(src->format) == dst->format))
      {
         uint32_t bpp = khrn_image_get_bpp(src->format);
         if (bpp == 32)
         {
            done = khrn_copy_8888_to_tf32(dst->storage, dst->width,
               dst_x, dst_y, width, height,
               srcStorage, src_x, src_y,
               src->stride, srcDir);
         }
         else if (bpp == 16)
         {
            done = khrn_copy_16_to_tf16(dst->storage, dst->width,
               dst_x, dst_y, width, height,
               src->storage, src_x, src_y,
               src->stride, srcDir);
         }
         else if (bpp == 8)
         {
            done = khrn_copy_8_to_tf8(dst->storage, dst->width,
               dst_x, dst_y, width, height,
               src->storage, src_x, src_y,
               src->stride, srcDir);
         }
      }
   }
#undef DST_FORMAT

   return done;
}

static bool khrn_part_fast_copy_to_tf(ABSTRACT_SERVER_ARGS_T *args)
{
   KHRN_FAST_COPY_TO_TF_ARGS_T *copyArgs = (KHRN_FAST_COPY_TO_TF_ARGS_T*)args->args;
   uint32_t part     = args->part;
   uint32_t numParts = args->numParts;

   uint32_t sy, dy, height;
   uint32_t partHeight = copyArgs->height / numParts;

   sy = copyArgs->src_y + (part * partHeight);
   dy = copyArgs->dst_y + (part * partHeight);

   height = partHeight;
   if (part == numParts - 1)
      height = copyArgs->height - (partHeight * part);

   if (height == 0)
      return true;

   return khrn_fast_copy_to_tf(copyArgs->dst, copyArgs->dst_x, dy, copyArgs->width, height,
                               copyArgs->src, copyArgs->src_x, sy, copyArgs->conv);
}

static bool khrn_par_fast_copy_to_tf(
   KHRN_IMAGE_WRAP_T *dst,
   uint32_t  dst_x, uint32_t dst_y,
   uint32_t  width, uint32_t height,
   const KHRN_IMAGE_WRAP_T *src,
   uint32_t  src_x, uint32_t src_y,
   KHRN_IMAGE_CONV_T conv)
{
   bool parallel = false;

   if (khrn_par_num_workers() > 1)
   {
      if (conv != IMAGE_CONV_GL)
         return false;
      else
      {
         /* Special case whole buffer with no format or colour-space conversion */
         /* All sizes and strides must match, buffers must be tightly packed    */
         /* Should be used e.g. when doing CopyTexImage2D() from an FBO         */
         if (khrn_image_is_tformat(src->format) &&
            src->format == dst->format &&
            dst_x == 0 && dst_y == 0 &&
            src_x == 0 && src_y == 0 &&
            width == src->width && height == src->height &&
            width == dst->width && height == dst->height &&
            src->stride == dst->stride &&
            dst->stride == (int32_t)khrn_image_get_stride(dst->format, width))
         {
            khrn_memcpy(dst->storage, src->storage, khrn_image_get_size(dst->format, width, height));
            return true;
         }

         if (width * height < 60 * 1024 || height < khrn_par_num_workers()) /* TODO : tune the cross-over */
         {
            parallel = false;
         }
         else
         {

#ifdef BIG_ENDIAN_CPU
#define DST_FORMAT RGBX_8888_TF
#else
#define DST_FORMAT XBGR_8888_TF
#endif
            if (((uint32_t)src->storage & 0x3) == 0 && is_stride_valid(src->format, src->stride))
            {
               if (src->format == YUV_422 || src->format == YUV_422_REV)
               {
                  parallel = (dst->format == DST_FORMAT) || (dst->format == YUV_422_RSO);
               }
               else if (src->format == BGR_888 && dst->format == XBGR_8888_TF)
               {
                  parallel = true;
               }
               else if (khrn_image_is_rso(src->format) && khrn_image_is_tformat(dst->format) &&
                  (khrn_image_to_tf_format(src->format) == dst->format))
               {
                  uint32_t bpp = khrn_image_get_bpp(src->format);
                  switch (bpp)
                  {
                  case 32:
                  case 16:
                  case 8:
                     parallel = true;
                     break;

                  default:
                     break;
                  }
               }
            }
#undef DST_FORMAT
         }
      }
   }

   if (parallel)
   {
      KHRN_FAST_COPY_TO_TF_ARGS_T   copyArgs;
      copyArgs.conv     = conv;
      copyArgs.dst      = dst;
      copyArgs.dst_x    = dst_x;
      copyArgs.dst_y    = dst_y;
      copyArgs.src      = src;
      copyArgs.src_x    = src_x;
      copyArgs.src_y    = src_y;
      copyArgs.width    = width;
      copyArgs.height   = height;

      return khrn_par_server_job(khrn_part_fast_copy_to_tf, &copyArgs);
   }
   else
      return khrn_fast_copy_to_tf(dst, dst_x, dst_y, width, height, src, src_x, src_y, conv);
}

void khrn_image_wrap_copy_region(
   KHRN_IMAGE_WRAP_T *dst, uint32_t dst_x, uint32_t dst_y,
   uint32_t width, uint32_t height,
   const KHRN_IMAGE_WRAP_T *src, uint32_t src_x, uint32_t src_y,
   KHRN_IMAGE_CONV_T conv)
{
   if ((dst->format == src->format) &&
      (src->flags == dst->flags) &&
      (dst->width == width) && (dst->height == height) &&
      (src->width == width) && (src->height == height) &&
      (dst->stride == (int32_t)khrn_image_get_stride(dst->format, width)) && /* only if the images are tightly packed... */
      (dst->stride == src->stride)) {
      assert((dst_x == 0) && (dst_y == 0) && (src_x == 0) && (src_y == 0));
      /* don't copy exactly over the top, as memcpy is undefined behavior */
      if (dst->storage != src->storage)
         khrn_memcpy(dst->storage, src->storage, khrn_image_get_size(dst->format, width, height));
      goto done;
   }

   assert((dst_x + width) <= dst->width);
   assert((dst_y + height) <= dst->height);
   assert((src_x + width) <= src->width);
   assert((src_y + height) <= src->height);

   if (khrn_image_is_etc1(dst->format) && khrn_image_is_etc1(src->format))
   {
      /*
         ETC1 repacking is an identical operation to RGBA32 repacking, where 4x4 ETC1 pixels are
         mapped to 2x1 ABGR pixels (64 bits in either case).
         Obviously we can't do the copy if the src/dst regions are not aligned to ETC1 blocks.
      */
      KHRN_IMAGE_WRAP_T rgba_dst, rgba_src;

      assert(!(dst_x & 3) && !(dst_y & 3) && !(src_x & 3) && !(src_y && 3));
      assert(!(width & 3) || dst_x + width == dst->width);
      assert(!(height & 3) || dst_y + height == dst->height);

      khrn_image_wrap(&rgba_dst,
         ABGR_8888 | (dst->format & IMAGE_FORMAT_MEM_LAYOUT_MASK),
         2*((dst->width+3)/4), (dst->height+3)/4,
         4 * dst->stride, dst->flags, false, dst->storage);
      khrn_image_wrap(&rgba_src,
         ABGR_8888 | (src->format & IMAGE_FORMAT_MEM_LAYOUT_MASK),
         2*((src->width+3)/4), (src->height+3)/4,
         4 * src->stride, src->flags, false, src->storage);
      khrn_image_wrap_copy_region(
         &rgba_dst, dst_x/4, dst_y/4, 2*((width+3)/4), (height+3)/4,
         &rgba_src, src_x/4, src_y/4, IMAGE_CONV_GL);
      goto done;
   }

   assert(khrn_image_is_uncomp(dst->format) && (khrn_image_is_uncomp(src->format) || (src->format == PACKED_MASK_TF)));

   if (khrn_bf_copy_region(dst, dst_x, dst_y, width, height, src, src_x, src_y, conv))
      goto done;

   /* Check for fast path special cases
    */

   if (khrn_par_fast_copy_to_tf(dst, dst_x, dst_y, width, height, src, src_x, src_y, conv))
   {
      INCR_DRIVER_COUNTER(tex_fast_paths);
      goto done;
   }

   if (src->format == PACKED_MASK_TF) {
      copy_region_from_packed_mask_tf(dst, dst_x, dst_y, width, height, src, src_x, src_y);
      goto done;
   }

   par_copy_scissor_regions(dst, dst_x, dst_y, width, height, src, src_x, src_y, conv, NULL, 0);
done:
   INCR_DRIVER_COUNTER(tex_submissions);
}


void khrn_image_wrap_copy_scissor_regions(
   KHRN_IMAGE_WRAP_T *dst, uint32_t dst_x, uint32_t dst_y,
   uint32_t width, uint32_t height,
   const KHRN_IMAGE_WRAP_T *src, uint32_t src_x, uint32_t src_y,
   KHRN_IMAGE_CONV_T conv,
   const int32_t *scissor_rects, uint32_t scissor_rects_count)
{
   if (scissor_rects_count == 4) {
      int32_t rect_x      = scissor_rects[0];
      int32_t rect_y      = scissor_rects[1];
      int32_t rect_width  = scissor_rects[2];
      int32_t rect_height = scissor_rects[3];
      khrn_clip_rect(
         &rect_x, &rect_y, &rect_width, &rect_height,
         dst_x, dst_y, width, height);
      khrn_image_wrap_copy_region(
         dst, rect_x, rect_y,
         rect_width, rect_height,
         src, src_x + (rect_x - dst_x), src_y + (rect_y - dst_y),
         conv);
      return;
   }

   assert(khrn_image_is_uncomp(dst->format) && khrn_image_is_uncomp(src->format));
   assert((dst_x + width) <= dst->width);
   assert((dst_y + height) <= dst->height);
   assert((src_x + width) <= src->width);
   assert((src_y + height) <= src->height);

   if (khrn_bf_copy_scissor_regions(dst, dst_x, dst_y, width, height, src, src_x, src_y, conv, scissor_rects, scissor_rects_count))
      return;

   par_copy_scissor_regions(dst, dst_x, dst_y, width, height, src, src_x, src_y, conv, scissor_rects, scissor_rects_count);
}

void khrn_image_wrap_convert(KHRN_IMAGE_WRAP_T *dst, const KHRN_IMAGE_WRAP_T *src, KHRN_IMAGE_CONV_T conv)
{
   assert((dst->width == src->width) && (dst->height == src->height));
   khrn_image_wrap_copy_region(dst, 0, 0, dst->width, dst->height, src, 0, 0, conv);
}

// This function takes two 32-bit depth/stencil images, assumed to be the same
// size, and copies the stencil channel of one onto the stencil channel of
// the other.
// TODO: make this fast! This is used when renderbuffers are recombined, so
// unlike glTexImage functions it could be considered to be on the fast path.
void khrn_image_wrap_copy_stencil_channel(KHRN_IMAGE_WRAP_T *dst, const KHRN_IMAGE_WRAP_T *src)
{
   uint32_t x, y;

   assert((dst->format == DEPTH_32_TF) && (src->format == DEPTH_32_TF));
   assert((dst->width == src->width) && (dst->height == src->height));

   for (y = 0; y != dst->height; ++y) {
      for (x = 0; x != dst->width; ++x) {
         uint32_t pixel0 = khrn_image_wrap_get_pixel(dst, x, y);
         uint32_t pixel1 = khrn_image_wrap_get_pixel(src, x, y);
         khrn_image_wrap_put_pixel(dst, x, y, (pixel0 & 0x00ffffff) | (pixel1 & 0xff000000));
      }
   }
}

static uint32_t blend(KHRN_IMAGE_FORMAT_T format, uint32_t x, uint32_t y)
{
   assert(khrn_image_is_color(format));

   switch (format & (IMAGE_FORMAT_PIXEL_SIZE_MASK | IMAGE_FORMAT_PIXEL_LAYOUT_MASK)) {
   case (IMAGE_FORMAT_32 | IMAGE_FORMAT_8888):
   {
      uint32_t a = ((x >> 24) + (y >> 24)) >> 1;
      uint32_t b = ((x >> 16 & 0xff) + (y >> 16 & 0xff)) >> 1;
      uint32_t g = ((x >> 8 & 0xff) + (y >> 8 & 0xff)) >> 1;
      uint32_t r = ((x & 0xff) + (y & 0xff)) >> 1;

      return a << 24 | b << 16 | g << 8 | r;
   }
   /* Miss out BGR888: currently no need to blend this type */
   case (IMAGE_FORMAT_16 | IMAGE_FORMAT_4444):
   {
      uint32_t r, g, b, a;

      assert(x < 65536);
      assert(y < 65536);

      r = ((x >> 12) + (y >> 12)) >> 1;
      g = ((x >> 8 & 0xf) + (y >> 8 & 0xf)) >> 1;
      b = ((x >> 4 & 0xf) + (y >> 4 & 0xf)) >> 1;
      a = ((x & 0xf) + (y & 0xf)) >> 1;

      return r << 12 | g << 8 | b << 4 | a;
   }
   case (IMAGE_FORMAT_16 | IMAGE_FORMAT_5551):
   {
      uint32_t r, g, b, a;

      assert(x < 65536);
      assert(y < 65536);

      assert(format & IMAGE_FORMAT_XA);

      r = ((x >> 11) + (y >> 11)) >> 1;
      g = ((x >> 6 & 0x1f) + (y >> 6 & 0x1f)) >> 1;
      b = ((x >> 1 & 0x1f) + (y >> 1 & 0x1f)) >> 1;
      a = ((x & 0x1) + (y & 0x1)) >> 1;

      return r << 11 | g << 6 | b << 1 | a;
   }
   case (IMAGE_FORMAT_16 | IMAGE_FORMAT_565):
   {
      uint32_t r, g, b;

      assert(x < 65536);
      assert(y < 65536);

      r = ((x >> 11) + (y >> 11)) >> 1;
      g = ((x >> 5 & 0x3f) + (y >> 5 & 0x3f)) >> 1;
      b = ((x & 0x1f) + (y & 0x1f)) >> 1;

      return r << 11 | g << 5 | b;
   }
   case (IMAGE_FORMAT_16 | IMAGE_FORMAT_88):
   {
      uint32_t a, l;

      assert(x < 65536);
      assert(y < 65536);

      a = ((x >> 8) + (y >> 8)) >> 1;
      l = ((x & 0xff) + (y & 0xff)) >> 1;

      return a << 8 | l;
   }
   case IMAGE_FORMAT_8:
   case IMAGE_FORMAT_4:
   case IMAGE_FORMAT_1:
   {
      return (x + y) >> 1;
   }
   default:
   {
      UNREACHABLE();
      return 0;
   }
   }
}

/* Blends four four-channel colours -- might be ABGR or RGBA */
static uint32_t blend_8888(uint32_t *ptr, uint32_t off)
{
   uint32_t rgba0 = ptr[off + 0];
   uint32_t rgba1 = ptr[off + 1];
   uint32_t rgba2 = ptr[off + 4];
   uint32_t rgba3 = ptr[off + 5];

   /* Filter the four colour samples */
   uint32_t a = ((rgba0 >> 24) + (rgba1 >> 24) + (rgba2 >> 24) + (rgba3 >> 24)) >> 2;
   uint32_t b = ((rgba0 & 0x00ff0000) + (rgba1 & 0x00ff0000) + (rgba2 & 0x00ff0000) + (rgba3 & 0x00ff0000)) >> 18;
   uint32_t g = ((rgba0 & 0x0000ff00) + (rgba1 & 0x0000ff00) + (rgba2 & 0x0000ff00) + (rgba3 & 0x0000ff00)) >> 10;
   uint32_t r = ((rgba0 & 0x000000ff) + (rgba1 & 0x000000ff) + (rgba2 & 0x000000ff) + (rgba3 & 0x000000ff)) >> 2;

   return (a << 24) | (b << 16) | (g << 8) | r;
}

/* Blends four three-channel colours held in 32-bits with top bits undefined ie. XBGR */
static uint32_t blend_x888(uint32_t *ptr, uint32_t off)
{
   uint32_t rgba0 = ptr[off + 0];
   uint32_t rgba1 = ptr[off + 1];
   uint32_t rgba2 = ptr[off + 4];
   uint32_t rgba3 = ptr[off + 5];

   /* Filter the four colour samples */
   uint32_t b = ((rgba0 & 0x00ff0000) + (rgba1 & 0x00ff0000) + (rgba2 & 0x00ff0000) + (rgba3 & 0x00ff0000)) >> 18;
   uint32_t g = ((rgba0 & 0x0000ff00) + (rgba1 & 0x0000ff00) + (rgba2 & 0x0000ff00) + (rgba3 & 0x0000ff00)) >> 10;
   uint32_t r = ((rgba0 & 0x000000ff) + (rgba1 & 0x000000ff) + (rgba2 & 0x000000ff) + (rgba3 & 0x000000ff)) >> 2;

   return (b << 16) | (g << 8) | r;
}

/* Blends four three-channel colours held in 32-bits with bottom bits undefined ie. RGBX */
static uint32_t blend_888x(uint32_t *ptr, uint32_t off)
{
   uint32_t rgba0 = ptr[off + 0];
   uint32_t rgba1 = ptr[off + 1];
   uint32_t rgba2 = ptr[off + 4];
   uint32_t rgba3 = ptr[off + 5];

   /* Filter the four colour samples */
   uint32_t r = ((rgba0 >> 24) + (rgba1 >> 24) + (rgba2 >> 24) + (rgba3 >> 24)) >> 2;
   uint32_t g = ((rgba0 & 0x00ff0000) + (rgba1 & 0x00ff0000) + (rgba2 & 0x00ff0000) + (rgba3 & 0x00ff0000)) >> 18;
   uint32_t b = ((rgba0 & 0x0000ff00) + (rgba1 & 0x0000ff00) + (rgba2 & 0x0000ff00) + (rgba3 & 0x0000ff00)) >> 10;

   return (g << 16) | (r << 24) | (b << 8);
}

/* Blends four 16-bit RGB channels */
static uint32_t blend_565(uint16_t *ptr, uint32_t off)
{
   uint32_t rgb0 = ptr[off + 0];
   uint32_t rgb1 = ptr[off + 1];
   uint32_t rgb2 = ptr[off + 8];
   uint32_t rgb3 = ptr[off + 9];

   /* Filter the four colour samples */
   uint32_t r = ((rgb0 & 0xf800) + (rgb1 & 0xf800) + (rgb2 & 0xf800) + (rgb3 & 0xf800)) >> 13;
   uint32_t g = ((rgb0 & 0x07e0) + (rgb1 & 0x07e0) + (rgb2 & 0x07e0) + (rgb3 & 0x07e0)) >> 7;
   uint32_t b = ((rgb0 & 0x001f) + (rgb1 & 0x001f) + (rgb2 & 0x001f) + (rgb3 & 0x001f)) >> 2;

   return (r << 11) | (g << 5) | b;
}

/* Blends four pairs of 8-bit channels -- might be LA or AL */
static uint32_t blend_88(uint16_t *ptr, uint32_t off)
{
   uint32_t al0 = ptr[off + 0];
   uint32_t al1 = ptr[off + 1];
   uint32_t al2 = ptr[off + 8];
   uint32_t al3 = ptr[off + 9];

   /* Filter the four colour samples */
   uint32_t a = ((al0 & 0xff00) + (al1 & 0xff00) + (al2 & 0xff00) + (al3 & 0xff00)) >> 10;
   uint32_t l = ((al0 & 0x00ff) + (al1 & 0x00ff) + (al2 & 0x00ff) + (al3 & 0x00ff)) >> 2;

   return (a << 8) | l;
}

/* Blends four 8-bit channels -- might be alpha or luminance */
static uint32_t blend_8(uint8_t *ptr, uint32_t off)
{
   uint32_t a0 = ptr[off + 0];
   uint32_t a1 = ptr[off + 1];
   uint32_t a2 = ptr[off + 8];
   uint32_t a3 = ptr[off + 9];

   /* Filter the four colour samples */
   uint32_t a = (a0 + a1 + a2 + a3) >> 2;

   return a;
}

static uint32_t  sample_offset32[16] =
{
   0x00, 0x02, 0x10, 0x12,
   0x08, 0x0a, 0x18, 0x1a,
   0x40, 0x42, 0x50, 0x52,
   0x48, 0x4a, 0x58, 0x5a
};

static const uint32_t  sample_offset16[32] =
{
   0x00, 0x02, 0x04, 0x06,
   0x20, 0x22, 0x24, 0x26,
   0x10, 0x12, 0x14, 0x16,
   0x30, 0x32, 0x34, 0x36,

   0x80, 0x82, 0x84, 0x86,
   0xa0, 0xa2, 0xa4, 0xa6,
   0x90, 0x92, 0x94, 0x96,
   0xb0, 0xb2, 0xb4, 0xb6
};

static const uint32_t  sample_offset8[64] =
{
   0x000, 0x002, 0x004, 0x006,
   0x040, 0x042, 0x044, 0x046,
   0x010, 0x012, 0x014, 0x016,
   0x050, 0x052, 0x054, 0x056,

   0x020, 0x022, 0x024, 0x026,
   0x060, 0x062, 0x064, 0x066,
   0x030, 0x032, 0x034, 0x036,
   0x070, 0x072, 0x074, 0x076,

   0x100, 0x102, 0x104, 0x106,
   0x140, 0x142, 0x144, 0x146,
   0x110, 0x112, 0x114, 0x116,
   0x150, 0x152, 0x154, 0x156,

   0x120, 0x122, 0x124, 0x126,
   0x160, 0x162, 0x164, 0x166,
   0x130, 0x132, 0x134, 0x136,
   0x170, 0x172, 0x174, 0x176,
};

static uint32_t ut_size_4(uint32_t inSize)
{
   uint32_t size = inSize;

   size = size >> 2;

   if ((inSize & 0x3) != 0)
      size += 1;

   return size;
}

static uint32_t ut_size_8(uint32_t inSize)
{
   uint32_t size = inSize;

   size = size >> 3;

   if ((inSize & 0x7) != 0)
      size += 1;

   return size;
}

static void subsample_8888_tf(KHRN_IMAGE_WRAP_T *dst, const KHRN_IMAGE_WRAP_T *src,
                              uint32_t part, uint32_t numParts)
{
   uint32_t x, y, t;

   /* Widths of the memory layout */
   uint32_t dstMicroWidthMem  = khrn_image_wrap_get_width_ut(dst);
   uint32_t srcMicroWidthMem  = khrn_image_wrap_get_width_ut(src);

   /* Actual width of image in micro tiles */
   uint32_t dstMicroWidth  = ut_size_4(dst->width);
   uint32_t dstMicroHeight = ut_size_4(dst->height);

   uint32_t sy, height;
   uint32_t partHeight = dstMicroHeight / numParts;

   sy = part * partHeight;

   height = partHeight;
   if (part == numParts - 1)
      height = dstMicroHeight - (partHeight * part);

   if (height == 0)
      return;

   for (y = sy; y != sy + height; ++y)
   {
      for (x = 0; x != dstMicroWidth; ++x)
      {
         uint32_t *srcTile = (uint32_t *)src->storage + (khrn_tformat_utile_addr(srcMicroWidthMem, x << 1, y << 1, true, NULL) << 4);
         uint32_t *dstTile = (uint32_t *)dst->storage + (khrn_tformat_utile_addr(dstMicroWidthMem, x,      y,      true, NULL) << 4);

         for (t = 0; t != 16; ++t)
            dstTile[t] = blend_8888(srcTile, sample_offset32[t]);
      }
   }
}

static void subsample_x888_tf(KHRN_IMAGE_WRAP_T *dst, const KHRN_IMAGE_WRAP_T *src,
                                 uint32_t part, uint32_t numParts)
{
   uint32_t x, y, t;

   /* Widths of the memory layout */
   uint32_t dstMicroWidthMem  = khrn_image_wrap_get_width_ut(dst);
   uint32_t srcMicroWidthMem  = khrn_image_wrap_get_width_ut(src);

   /* Actual width of image in micro tiles */
   uint32_t dstMicroWidth  = ut_size_4(dst->width);
   uint32_t dstMicroHeight = ut_size_4(dst->height);

   uint32_t sy, height;
   uint32_t partHeight = dstMicroHeight / numParts;

   sy = part * partHeight;

   height = partHeight;
   if (part == numParts - 1)
      height = dstMicroHeight - (partHeight * part);

   if (height == 0)
      return;

   for (y = sy; y != sy + height; ++y)
   {
      for (x = 0; x != dstMicroWidth; ++x)
      {
         uint32_t *srcTile = (uint32_t *)src->storage + (khrn_tformat_utile_addr(srcMicroWidthMem, x << 1, y << 1, true, NULL) << 4);
         uint32_t *dstTile = (uint32_t *)dst->storage + (khrn_tformat_utile_addr(dstMicroWidthMem, x,      y,      true, NULL) << 4);

         for (t = 0; t != 16; ++t)
            dstTile[t] = blend_x888(srcTile, sample_offset32[t]);
      }
   }
}

static void subsample_888x_tf(KHRN_IMAGE_WRAP_T *dst, const KHRN_IMAGE_WRAP_T *src,
                              uint32_t part, uint32_t numParts)
{
   uint32_t x, y, t;

   /* Widths of the memory layout */
   uint32_t dstMicroWidthMem  = khrn_image_wrap_get_width_ut(dst);
   uint32_t srcMicroWidthMem  = khrn_image_wrap_get_width_ut(src);

   /* Actual width of image in micro tiles */
   uint32_t dstMicroWidth  = ut_size_4(dst->width);
   uint32_t dstMicroHeight = ut_size_4(dst->height);

   uint32_t sy, height;
   uint32_t partHeight = dstMicroHeight / numParts;

   sy = part * partHeight;

   height = partHeight;
   if (part == numParts - 1)
      height = dstMicroHeight - (partHeight * part);

   if (height == 0)
      return;

   for (y = sy; y != sy + height; ++y)
   {
      for (x = 0; x != dstMicroWidth; ++x)
      {
         uint32_t *srcTile = (uint32_t *)src->storage + (khrn_tformat_utile_addr(srcMicroWidthMem, x << 1, y << 1, true, NULL) << 4);
         uint32_t *dstTile = (uint32_t *)dst->storage + (khrn_tformat_utile_addr(dstMicroWidthMem, x,      y,      true, NULL) << 4);

         for (t = 0; t != 16; ++t)
            dstTile[t] = blend_888x(srcTile, sample_offset32[t]);
      }
   }
}


static void subsample_565_tf(KHRN_IMAGE_WRAP_T *dst, const KHRN_IMAGE_WRAP_T *src,
                             uint32_t part, uint32_t numParts)
{
   uint32_t x, y, t;

   /* Widths of the memory layout */
   uint32_t dstMicroWidthMem  = khrn_image_wrap_get_width_ut(dst);
   uint32_t srcMicroWidthMem  = khrn_image_wrap_get_width_ut(src);

   /* Actual width of image in micro tiles */
   uint32_t dstMicroWidth  = ut_size_8(dst->width);
   uint32_t dstMicroHeight = ut_size_4(dst->height);

   uint32_t sy, height;
   uint32_t partHeight = dstMicroHeight / numParts;

   sy = part * partHeight;

   height = partHeight;
   if (part == numParts - 1)
      height = dstMicroHeight - (partHeight * part);

   if (height == 0)
      return;

   for (y = sy; y != sy + height; ++y)
   {
      for (x = 0; x != dstMicroWidth; ++x)
      {
         uint16_t *srcTile = (uint16_t *)src->storage + (khrn_tformat_utile_addr(srcMicroWidthMem, x << 1, y << 1, true, NULL) << 5);
         uint16_t *dstTile = (uint16_t *)dst->storage + (khrn_tformat_utile_addr(dstMicroWidthMem, x,      y,      true, NULL) << 5);

         for (t = 0; t != 32; ++t)
            dstTile[t] = blend_565(srcTile, sample_offset16[t]);
      }
   }
}

static void subsample_88_tf(KHRN_IMAGE_WRAP_T *dst, const KHRN_IMAGE_WRAP_T *src,
                            uint32_t part, uint32_t numParts)
{
   uint32_t x, y, t;

   /* Widths of the memory layout */
   uint32_t dstMicroWidthMem  = khrn_image_wrap_get_width_ut(dst);
   uint32_t srcMicroWidthMem  = khrn_image_wrap_get_width_ut(src);

   /* Actual width of image in micro tiles */
   uint32_t dstMicroWidth  = ut_size_8(dst->width);
   uint32_t dstMicroHeight = ut_size_4(dst->height);

   uint32_t sy, height;
   uint32_t partHeight = dstMicroHeight / numParts;

   sy = part * partHeight;

   height = partHeight;
   if (part == numParts - 1)
      height = dstMicroHeight - (partHeight * part);

   if (height == 0)
      return;

   for (y = sy; y != sy + height; ++y)
   {
      for (x = 0; x != dstMicroWidth; ++x)
      {
         uint16_t *srcTile = (uint16_t *)src->storage + (khrn_tformat_utile_addr(srcMicroWidthMem, x << 1, y << 1, true, NULL) << 5);
         uint16_t *dstTile = (uint16_t *)dst->storage + (khrn_tformat_utile_addr(dstMicroWidthMem, x,      y,      true, NULL) << 5);

         for (t = 0; t != 32; ++t)
            dstTile[t] = blend_88(srcTile, sample_offset16[t]);
      }
   }
}

static void subsample_8_tf(KHRN_IMAGE_WRAP_T *dst, const KHRN_IMAGE_WRAP_T *src,
                           uint32_t part, uint32_t numParts)
{
   uint32_t x, y, t;
   uint8_t  *src_addr = (uint8_t *)src->storage;
   uint8_t  *dst_addr = (uint8_t *)dst->storage;

   /* Widths of the memory layout */
   uint32_t dstMicroWidthMem  = khrn_image_wrap_get_width_ut(dst);
   uint32_t srcMicroWidthMem  = khrn_image_wrap_get_width_ut(src);

   /* Actual width of image in micro tiles */
   uint32_t dstMicroWidth  = ut_size_8(dst->width);
   uint32_t dstMicroHeight = ut_size_8(dst->height);

   uint32_t sy, height;
   uint32_t partHeight = dstMicroHeight / numParts;

   sy = part * partHeight;

   height = partHeight;
   if (part == numParts - 1)
      height = dstMicroHeight - (partHeight * part);

   if (height == 0)
      return;

   for (y = sy; y != sy + height; ++y)
   {
      for (x = 0; x != dstMicroWidth; ++x)
      {
         uint8_t *srcTile = src_addr + (khrn_tformat_utile_addr(srcMicroWidthMem, x << 1, y << 1, true, NULL) << 6);
         uint8_t *dstTile = dst_addr + (khrn_tformat_utile_addr(dstMicroWidthMem, x,      y     , true, NULL) << 6);

         for (t = 0; t != 64; t += 4)
         {
            /* Batch writes into words */
            uint32_t   a = blend_8(srcTile, sample_offset8[t + 0]);
            uint32_t   b = blend_8(srcTile, sample_offset8[t + 1]);
            uint32_t   c = blend_8(srcTile, sample_offset8[t + 2]);
            uint32_t   d = blend_8(srcTile, sample_offset8[t + 3]);

#ifdef BIG_ENDIAN_CPU
            *(uint32_t *)(&dstTile[t]) = a << 24 | b << 16 | c << 8 | d;
#else
            *(uint32_t *)(&dstTile[t]) = d << 24 | c << 16 | b << 8 | a;
#endif
         }
      }
   }
}

static bool part_image_wrap_subsample_fast(ABSTRACT_SERVER_ARGS_T *args)
{
   KHRN_WRAP_SUBSAMPLE_FAST_ARGS_T  *subArgs = (KHRN_WRAP_SUBSAMPLE_FAST_ARGS_T*)args->args;
   KHRN_IMAGE_WRAP_T                *dst = subArgs->dst;
   const KHRN_IMAGE_WRAP_T          *src = subArgs->src;
   uint32_t                         part = args->part;
   uint32_t                         numParts = args->numParts;

   if (src->format != dst->format)
   {
      subArgs->retCode = false;
      return false;
   }

   if ((src->format & IMAGE_FORMAT_TF) == 0)
   {
      subArgs->retCode = false;
      return false;
   }

   switch (src->format)
   {
   case ABGR_8888_TF:
   case RGBA_8888_TF:
      subsample_8888_tf(dst, src, part, numParts);
      break;

   case XBGR_8888_TF:
      subsample_x888_tf(dst, src, part, numParts);
      break;

   case RGBX_8888_TF:
      subsample_888x_tf(dst, src, part, numParts);
      break;

   case RGB_565_TF:
   case BGR_565_TF:
      subsample_565_tf(dst, src, part, numParts);
      break;

   case AL_88_TF:
   case LA_88_TF:
      subsample_88_tf(dst, src, part, numParts);
      break;

   case A_8_TF:
   case L_8_TF:
      subsample_8_tf(dst, src, part, numParts);
      break;

   default:
      subArgs->retCode = false;
      return false;
      break;
   }

   return true;
}

/* image_wrap_subsample_fast
 *
 * Select the appropriate fast path for 2x2 downsampling a tf image to another
 * tf image
 * Returns false if there is no fast path.
 */
static bool image_wrap_subsample_fast(KHRN_IMAGE_WRAP_T *dst, const KHRN_IMAGE_WRAP_T *src)
{
   bool done = true;
   bool parallel = false;

   if (src->format != dst->format)
      return false;

   if ((src->format & IMAGE_FORMAT_TF) == 0)
      return false;

   if (khrn_par_num_workers() > 1 && src->height > khrn_par_num_workers() * 4)
      parallel = true;

   if (parallel && khrn_par_num_workers() > 1)
   {
      switch (src->format)
      {
      case ABGR_8888_TF:
      case RGBA_8888_TF:
      case XBGR_8888_TF:
      case RGBX_8888_TF:
      case RGB_565_TF:
      case BGR_565_TF:
      case AL_88_TF:
      case LA_88_TF:
      case A_8_TF:
      case L_8_TF:
         {
            KHRN_WRAP_SUBSAMPLE_FAST_ARGS_T  parArgs;
            parArgs.dst      = dst;
            parArgs.src      = src;
            return khrn_par_server_job(part_image_wrap_subsample_fast, &parArgs);
         }
      default:
         return false;
      }
   }
   else
   {
      switch (src->format)
      {
      case ABGR_8888_TF:
      case RGBA_8888_TF:
         subsample_8888_tf(dst, src, 0, 1);
         break;

      case XBGR_8888_TF:
         subsample_x888_tf(dst, src, 0, 1);
         break;

      case RGBX_8888_TF:
         subsample_888x_tf(dst, src, 0, 1);
         break;

      case RGB_565_TF:
      case BGR_565_TF:
         subsample_565_tf(dst, src, 0, 1);
         break;

      case AL_88_TF:
      case LA_88_TF:
         subsample_88_tf(dst, src, 0, 1);
         break;

      case A_8_TF:
      case L_8_TF:
         subsample_8_tf(dst, src, 0, 1);
         break;

      default:
         done = false;
         break;
      }
   }

   return done;
}

void khrn_image_wrap_subsample(KHRN_IMAGE_WRAP_T *dst, const KHRN_IMAGE_WRAP_T *src)
{
   uint32_t x, y;

   assert(!((dst->format ^ src->format) & ~IMAGE_FORMAT_MEM_LAYOUT_MASK & ~IMAGE_FORMAT_PRE));
   assert(khrn_image_is_color(src->format));
   assert(dst->width == _max(src->width >> 1, 1));
   assert(dst->height == _max(src->height >> 1, 1));

   if (khrn_bf_subsample(dst, src))
      goto done;

   if (image_wrap_subsample_fast(dst, src))
   {
      INCR_DRIVER_COUNTER(mipmap_gens_fast);
      goto done;
   }

   for (y = 0; y != dst->height; ++y) {
      for (x = 0; x != dst->width; ++x) {
         uint32_t sx0 = x << 1;
         uint32_t sy0 = y << 1;
         uint32_t sx1 = _min(sx0 + 1, (uint32_t)src->width - 1);
         uint32_t sy1 = _min(sy0 + 1, (uint32_t)src->height - 1);

         uint32_t p0 = khrn_image_wrap_get_pixel(src, sx0, sy0);
         uint32_t p1 = khrn_image_wrap_get_pixel(src, sx1, sy0);
         uint32_t p2 = khrn_image_wrap_get_pixel(src, sx0, sy1);
         uint32_t p3 = khrn_image_wrap_get_pixel(src, sx1, sy1);

         uint32_t p = blend(src->format, blend(src->format, p0, p1), blend(src->format, p2, p3));

         khrn_image_wrap_put_pixel(dst, x, y, p);
      }
   }
done:
   INCR_DRIVER_COUNTER(mipmap_gens);
}

void khrn_image_clear_region(
   KHRN_IMAGE_T *image, uint32_t x, uint32_t y,
   uint32_t width, uint32_t height,
   uint32_t rgba, /* non-lin, unpre */
   KHRN_IMAGE_CONV_T conv)
{
   KHRN_IMAGE_WRAP_T wrap;
   khrn_image_lock_wrap(image, &wrap);
   khrn_image_wrap_clear_region(&wrap, x, y, width, height, rgba, conv);
   khrn_image_unlock_wrap(image);
}

void khrn_image_copy_region(
   KHRN_IMAGE_T *dst, uint32_t dst_x, uint32_t dst_y,
   uint32_t width, uint32_t height,
   const KHRN_IMAGE_T *src, uint32_t src_x, uint32_t src_y,
   KHRN_IMAGE_CONV_T conv)
{
   KHRN_IMAGE_WRAP_T dst_wrap, src_wrap;
   khrn_image_lock_wrap(dst, &dst_wrap);
   khrn_image_lock_wrap((KHRN_IMAGE_T *)src, &src_wrap);
   khrn_image_wrap_copy_region(&dst_wrap, dst_x, dst_y, width, height, &src_wrap, src_x, src_y, conv);
   khrn_image_unlock_wrap(src);
   khrn_image_unlock_wrap(dst);
}

void khrn_image_convert(KHRN_IMAGE_T *dst, const KHRN_IMAGE_T *src, KHRN_IMAGE_CONV_T conv)
{
   assert((dst->width == src->width) && (dst->height == src->height));
   khrn_image_copy_region(dst, 0, 0, dst->width, dst->height, src, 0, 0, conv);
}

void khrn_image_copy_stencil_channel(KHRN_IMAGE_T *dst, const KHRN_IMAGE_T *src)
{
   KHRN_IMAGE_WRAP_T dst_wrap, src_wrap;
   khrn_image_lock_wrap(dst, &dst_wrap);
   khrn_image_lock_wrap((KHRN_IMAGE_T *)src, &src_wrap);
   khrn_image_wrap_copy_stencil_channel(&dst_wrap, &src_wrap);
   khrn_image_unlock_wrap(src);
   khrn_image_unlock_wrap(dst);
}

void khrn_image_subsample(KHRN_IMAGE_T *dst, const KHRN_IMAGE_T *src)
{
   KHRN_IMAGE_WRAP_T dst_wrap, src_wrap;
   khrn_image_lock_wrap(dst, &dst_wrap);
   khrn_image_lock_wrap((KHRN_IMAGE_T *)src, &src_wrap);
   khrn_image_wrap_subsample(&dst_wrap, &src_wrap);
   khrn_image_unlock_wrap(src);
   khrn_image_unlock_wrap(dst);
}

bool khrn_image_alloc_storage(KHRN_IMAGE_T *image, const char *description)
{
   uint32_t storage_size;
   MEM_HANDLE_T handle;

   if (image->mh_storage == MEM_HANDLE_INVALID)
   {
      KHRN_IMAGE_FORMAT_T format = image->format;
      uint32_t padded_width = image->width, padded_height = image->height, align = DEFAULT_ALIGN;

      KHRN_IMAGE_CREATE_FLAG_T image_create_flags =
         ((image->flags & IMAGE_FLAG_TEXTURE) ? IMAGE_CREATE_FLAG_TEXTURE : 0) |
         ((image->flags & IMAGE_FLAG_RSO_TEXTURE) ? IMAGE_CREATE_FLAG_RSO_TEXTURE : 0) |
         ((image->flags & IMAGE_FLAG_RENDER_TARGET) ? IMAGE_CREATE_FLAG_RENDER_TARGET : 0) |
         ((image->flags & IMAGE_FLAG_DISPLAY) ? IMAGE_CREATE_FLAG_DISPLAY : 0);

      khrn_image_platform_fudge(&format, &padded_width, &padded_height, &align, image_create_flags);

      storage_size = khrn_image_get_size(image->format, padded_width, padded_height);

      /* TODO: I'm ignoring stagger/rotate here (see khrn_image_create calculation of storage_size) */
      MEM_FLAG_T mem_flag =
         MEM_FLAG_DIRECT | MEM_FLAG_NO_INIT | MEM_FLAG_RESIZEABLE;

      if (image->secure)
         mem_flag |= MEM_FLAG_SECURE;

      handle = mem_alloc_ex(storage_size, 4096,
         mem_flag,
         description,
         MEM_COMPACT_DISCARD);

      if (handle == MEM_HANDLE_INVALID)
         return false;

      MEM_ASSIGN(image->mh_storage, handle);
      mem_release(handle);

#ifdef DEBUG
      /* secure can't hit this buffer with CPU, so ignore */
      if (!khrn_image_is_secure(image->format))
      {
         /* Fill image with known color rather than snow */
         if (khrn_image_is_color(image->format) && (khrn_image_is_rso(image->format) ||
            khrn_image_is_tformat(image->format) || khrn_image_is_lineartile(image->format)))
            khrn_image_clear_region(image, 0, 0, image->width, image->height, 0xff0080ff, IMAGE_CONV_GL);
      }
#endif
   }
   return true;
}
