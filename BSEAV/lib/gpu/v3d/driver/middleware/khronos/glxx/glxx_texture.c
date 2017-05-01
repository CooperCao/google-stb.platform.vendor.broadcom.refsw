/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "interface/khronos/common/khrn_int_common.h"
#include "interface/khronos/glxx/glxx_int_config.h"
#include "interface/khronos/common/khrn_int_util.h"
#include "interface/khronos/include/GLES/gl.h"
#include "interface/khronos/include/GLES/glext.h"
#include "middleware/khronos/ext/egl_khr_image.h"
#include "middleware/khronos/common/khrn_hw.h"
#include "middleware/khronos/glxx/glxx_texture.h"
#include "middleware/khronos/common/khrn_image.h"
#include "middleware/khronos/common/khrn_interlock.h"
#include "middleware/khronos/glxx/glxx_server.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

static uint32_t texture_get_required_blob_size(GLXX_TEXTURE_T *texture, bool depaletted);
static uint32_t texture_get_mipmap_size(GLXX_TEXTURE_T *texture, uint32_t level, bool depaletted);
static uint32_t texture_get_mipmap_stride(GLXX_TEXTURE_T *texture, uint32_t level, bool depaletted);
static bool texture_unbind(GLXX_TEXTURE_T *texture);
static void texture_write_image_immediate(GLXX_TEXTURE_T *texture, uint32_t buffer, uint32_t level);
static uint32_t get_size(KHRN_IMAGE_FORMAT_T format, uint32_t width, uint32_t height);
static MEM_HANDLE_T * texture_get_storage_handle_ref(GLXX_TEXTURE_T *texture);
static bool keep_explicit_mipmaps(GLXX_TEXTURE_T *texture);

#ifndef NDEBUG
static void check_buffer(int buffer, const GLXX_TEXTURE_T *texture)
{
    bool ok = false;

    switch (texture->target) {
    case GL_TEXTURE_2D:
        ok = buffer == TEXTURE_BUFFER_TWOD;
        break;
    case GL_TEXTURE_EXTERNAL_OES:
        ok = buffer == TEXTURE_BUFFER_EXTERNAL;
        break;
    default:
        ok = buffer >= TEXTURE_BUFFER_POSITIVE_X
            && buffer <= TEXTURE_BUFFER_NEGATIVE_Z;
        break;
    }
    vcos_assert(ok);
}
#define CHECK_BUFFER(buffer, texture) do { check_buffer((buffer), \
        (texture)); } while (0)
#else
#define CHECK_BUFFER(buffer, texture) do {} while (0)
#endif

void assign_retain(MEM_HANDLE_T *x, MEM_HANDLE_T y)
{
   if(y!=MEM_HANDLE_INVALID)
      mem_acquire_retain(y);
   if(*x != MEM_HANDLE_INVALID)
   {
      if(mem_get_flags(*x) & MEM_FLAG_DISCARDABLE)
         mem_unretain(*x);
      mem_release(*x);/* release after unretain */
   }
   *x = y;
}

/*
   void glxx_texture_init(GLXX_TEXTURE_T *texture, int32_t name)

   initialize a texture object

   Implementation notes:

   THIS FUNCTION IS NOT YET REVIEWED

   Precondition:

   texture is a valid pointer to an uninitialised GLXX_TEXTURE_T
   memory management invariants for texture are satisfied
   name != 0

   Postcondition:

   The GLXX_TEXTURE_T pointed to by buffer is valid
*/

void glxx_texture_init(GLXX_TEXTURE_T *texture, int32_t name, GLenum target)
{
   int i, j;

   vcos_assert(texture);

   texture->name = name;
   texture->target = target;

   texture->min = GL_NEAREST_MIPMAP_LINEAR;
   texture->mag = GL_LINEAR;

   texture->wrap.s = GL_REPEAT;
   texture->wrap.t = GL_REPEAT;

   texture->generate_mipmap = GL_FALSE;

   for (i = 0; i < TEXTURE_BUFFER_COUNT; i++) {
      for (j = 0; j <= LOG2_MAX_TEXTURE_SIZE; j++)
         vcos_assert(texture->mh_mipmaps[i][j] == MEM_INVALID_HANDLE);
      texture->blob_valid[i] = 0;
   }
   texture->explicit_mipmaps = 0;
   texture->framebuffer_sharing = false;

   texture->current_item = 0;

   for (i = 0; i < GLXX_TEXTURE_POOL_SIZE; i++)
   {
      vcos_assert(texture->blob_pool[i].mh_storage == MEM_INVALID_HANDLE);
      texture->blob_pool[i].secure = false;
      khrn_interlock_init(&texture->blob_pool[i].interlock);
   }
   vcos_assert(texture->mh_depaletted_blob == MEM_INVALID_HANDLE);

   texture->binding_type = TEXTURE_BLOB_NULL;
   texture->mipmaps_available = true;

   texture->mip0_offset = ~0;

   MEM_ASSIGN(texture->mh_ms_image, MEM_INVALID_HANDLE);
}

static void unretain_current(GLXX_TEXTURE_T *texture)
{
   if(texture->blob_pool[texture->current_item].mh_storage!=MEM_INVALID_HANDLE
      && mem_get_flags(texture->blob_pool[texture->current_item].mh_storage) & MEM_FLAG_DISCARDABLE)
   {
      mem_unretain(texture->blob_pool[texture->current_item].mh_storage);
   }
}

static void nullify(GLXX_TEXTURE_T *texture)
{
   int i, j;

   for (i = 0; i < TEXTURE_BUFFER_COUNT; i++) {
      for (j = 0; j <= LOG2_MAX_TEXTURE_SIZE; j++)
         MEM_ASSIGN(texture->mh_mipmaps[i][j], MEM_INVALID_HANDLE);
      texture->blob_valid[i] = 0;
   }

   texture->framebuffer_sharing = false;

   unretain_current(texture);

   texture->explicit_mipmaps = 0;

   for (i = 0; i < GLXX_TEXTURE_POOL_SIZE; i++)
   {
      texture->blob_pool[i].secure = false;
      MEM_ASSIGN(texture->blob_pool[i].mh_storage, MEM_INVALID_HANDLE);
   }

   MEM_ASSIGN(texture->mh_depaletted_blob, MEM_INVALID_HANDLE);

   texture->binding_type = TEXTURE_BLOB_NULL;

   MEM_ASSIGN(texture->mh_ms_image, MEM_INVALID_HANDLE);

   MEM_ASSIGN(texture->external_image, MEM_INVALID_HANDLE);
}

void glxx_texture_term(MEM_HANDLE_T handle)
{
   int i;
   GLXX_TEXTURE_T *texture = (GLXX_TEXTURE_T *)mem_lock(handle, NULL);

   glxx_texture_release_teximage(texture);
   nullify(texture);

   for (i = 0; i < GLXX_TEXTURE_POOL_SIZE; i++)
   {
      khrn_interlock_term(&texture->blob_pool[i].interlock);
   }

   khrn_interlock_term(&texture->interlock);

   mem_unlock(handle);
}

static KHRN_IMAGE_FORMAT_T convert_format(GLenum format, GLenum type)
{
   switch (format) {
   case GL_RGBA:
      switch (type) {
      case GL_UNSIGNED_BYTE:
         #ifdef BIG_ENDIAN_CPU
            return RGBA_8888_TF;
         #else
            return ABGR_8888_TF;
         #endif
      case GL_UNSIGNED_SHORT_4_4_4_4:
         return RGBA_4444_TF;
      case GL_UNSIGNED_SHORT_5_5_5_1:
         return RGBA_5551_TF;
      default:
         UNREACHABLE();
         break;
      }
      break;
   case GL_RGB:
      switch (type) {
      case GL_UNSIGNED_BYTE:
         #ifdef BIG_ENDIAN_CPU
            return RGBX_8888_TF;
         #else
            return XBGR_8888_TF;
         #endif
      case GL_UNSIGNED_SHORT_5_6_5:
         return RGB_565_TF;
      default:
         UNREACHABLE();
         break;
      }
      break;
   case GL_LUMINANCE_ALPHA:
#ifdef BIG_ENDIAN_CPU
      return LA_88_TF;
#else
      return AL_88_TF;
#endif
   case GL_LUMINANCE:
      return L_8_TF;
   case GL_ALPHA:
      return A_8_TF;
#if GL_EXT_texture_format_BGRA8888
   case GL_BGRA_EXT:
      assert(type == GL_UNSIGNED_BYTE);
#ifdef BIG_ENDIAN_CPU
      return BGRA_8888_TF;
#else
      return ARGB_8888_TF;
#endif
#endif
#if GL_APPLE_rgb_422
   case GL_RGB_422_APPLE:
      assert(type == GL_UNSIGNED_SHORT_8_8_APPLE || type == GL_UNSIGNED_SHORT_8_8_REV_APPLE);
      /* If you return YUV_422_RSO here, we will texture directly from YUV_RSO, but we choose to
         t-format convert instead. If you texture using a YUV EGLImage, that will use YUV_RSO directly. */
#ifdef BIG_ENDIAN_CPU
      return RGBX_8888_TF;
#else
      return XBGR_8888_TF;
#endif
#endif

   default:
      UNREACHABLE();
      break;
   }

   return IMAGE_FORMAT_INVALID;
}

static KHRN_IMAGE_FORMAT_T convert_raster_format(GLenum format, GLenum type)
{
   switch (format) {
   case GL_RGBA:
      switch (type) {
      case GL_UNSIGNED_BYTE:
         #ifdef BIG_ENDIAN_CPU
            return RGBA_8888_RSO;
         #else
            return ABGR_8888_RSO;
         #endif
      case GL_UNSIGNED_SHORT_4_4_4_4:
         return RGBA_4444_RSO;
      case GL_UNSIGNED_SHORT_5_5_5_1:
         return RGBA_5551_RSO;
      default:
         UNREACHABLE();
         break;
      }
      break;
   case GL_RGB:
      switch (type) {
      case GL_UNSIGNED_BYTE:
         return BGR_888_RSO;
      case GL_UNSIGNED_SHORT_5_6_5:
         return RGB_565_RSO;
      default:
         UNREACHABLE();
         break;
      }
      break;
   case GL_LUMINANCE_ALPHA:
      #ifdef BIG_ENDIAN_CPU
         return LA_88_RSO;
      #else
         return AL_88_RSO;
      #endif
   case GL_LUMINANCE:
      return L_8_RSO;
   case GL_ALPHA:
      return A_8_RSO;
#if GL_EXT_texture_format_BGRA8888
   case GL_BGRA_EXT:
      vcos_assert(type == GL_UNSIGNED_BYTE);
#ifdef BIG_ENDIAN_CPU
      return BGRA_8888_RSO;
#else
      return ARGB_8888_RSO;
#endif
#endif
#if GL_APPLE_rgb_422
   case GL_RGB_422_APPLE:
      vcos_assert(type == GL_UNSIGNED_SHORT_8_8_APPLE || type == GL_UNSIGNED_SHORT_8_8_REV_APPLE);
      if (type == GL_UNSIGNED_SHORT_8_8_APPLE)
         return YUV_422_RSO;
      else if (type == GL_UNSIGNED_SHORT_8_8_REV_APPLE)
         return YUV_422_REV_RSO;
#endif
   default:
      UNREACHABLE();
      break;
   }

   return IMAGE_FORMAT_INVALID;
}

static KHRN_IMAGE_FORMAT_T convert_to_lt_format(KHRN_IMAGE_FORMAT_T format)
{
   switch (format) {
   case BGR_888_RSO:
      #ifdef BIG_ENDIAN_CPU
         return RGBX_8888_LT;
      #else
         return XBGR_8888_LT;
      #endif
   default:
      return khrn_image_to_lt_format(format);
   }

   return IMAGE_FORMAT_INVALID;
}

/*
   from section 3.6 of the OpenGL ES 1.1 spec

   the first element of the Nth row is indicated by

   p + Nk

   where N is the row number (counting from zero) and k is defined as

   k = nl                  if s >= a
     = a/s * ceil(snl/a)   otherwise

   where n is the number of elements in a group, l is the number of groups in
   the row, a is the value of UNPACK ALIGNMENT, and s is the size, in units of GL
   ubytes, of an element.
*/

static uint32_t get_stride(uint32_t l, GLenum format, GLenum type, uint32_t a)
{
   uint32_t n = 0, s = 0, k;

   switch (format) {
   case GL_RGBA:
#if GL_EXT_texture_format_BGRA8888
   case GL_BGRA_EXT:
#endif
      switch (type) {
      case GL_UNSIGNED_BYTE:
         n = 4;
         s = 1;
         break;
      case GL_UNSIGNED_SHORT_4_4_4_4:
      case GL_UNSIGNED_SHORT_5_5_5_1:
         n = 1;
         s = 2;
         break;
      default:
         UNREACHABLE();
         break;
      }
      break;
   case GL_RGB:
      switch (type) {
      case GL_UNSIGNED_BYTE:
         n = 3;
         s = 1;
         break;
      case GL_UNSIGNED_SHORT_5_6_5:
         n = 1;
         s = 2;
         break;
      default:
         UNREACHABLE();
         break;
      }
      break;
   case GL_LUMINANCE_ALPHA:
      n = 2;
      s = 1;
      break;
   case GL_LUMINANCE:
   case GL_ALPHA:
      n = 1;
      s = 1;
      break;
#if GL_APPLE_rgb_422
   case GL_RGB_422_APPLE:
      n = 1;
      s = 2;
      break;
#endif
   default:
      UNREACHABLE();
      break;
   }

   if (s < a)
      k = a * ((s * n * l + a - 1) / a);
   else
      k = s * n * l;

   return k;
}

static bool invalidate_ms_image(GLXX_TEXTURE_T *texture, int width, int height, bool secure)
{
   // recreate the ms image if it existed
   if (texture->mh_ms_image) {
      MEM_HANDLE_T hmsimage = glxx_image_create_ms(COL_32_TLBD,
         width, height,
         IMAGE_CREATE_FLAG_RENDER_TARGET | IMAGE_CREATE_FLAG_ONE, secure);

      if (hmsimage == MEM_INVALID_HANDLE)
         return false;

      MEM_ASSIGN(texture->mh_ms_image, hmsimage);
      mem_release(hmsimage);
   }

   return true;
}

static bool check_image(GLXX_TEXTURE_T *texture, uint32_t buffer, uint32_t level,
   KHRN_IMAGE_FORMAT_T format, int width, int height, bool secure)
{
   MEM_HANDLE_T himage;

   //Always specify the t-format version. It will get converted to linear tile
   //if necessary.
   vcos_assert(buffer < TEXTURE_BUFFER_COUNT);
#ifndef NDEBUG
   switch (buffer) {
   case TEXTURE_BUFFER_TWOD:
       vcos_assert(texture->target == GL_TEXTURE_2D);
       break;
   case TEXTURE_BUFFER_EXTERNAL:
       vcos_assert(texture->target == GL_TEXTURE_EXTERNAL_OES);
       break;
   default:
       vcos_assert(texture->target == GL_TEXTURE_CUBE_MAP);
       vcos_assert(width == height);
       break;
   }
#endif
   vcos_assert(level <= LOG2_MAX_TEXTURE_SIZE);
   vcos_assert(khrn_image_is_tformat(format) || khrn_image_is_yuv422(format));
   vcos_assert(width > 0 && height > 0);

   /*
      First case: This is the first level 0 image we have received, which
      therefore determines the expected overall texture size and format
   */
   if (texture->binding_type == TEXTURE_BLOB_NULL && level == 0) {
      int size, align;
      MEM_HANDLE_T handle;

      texture->binding_type = TEXTURE_STATE_UNDER_CONSTRUCTION;  /* need to set these first so texture_get_required_blob_size works */
      texture->width = width;
      texture->height = height;
      if (!khrn_image_is_yuv422(format))
         texture->format = khrn_image_to_lt_format(format);  /* _LT: mipmaps are a mixture of TF and LT. */
      else
         texture->format = format;
      texture->blob_mip_count = glxx_texture_get_mipmap_count(texture);
      texture->offset = 0;
      size = texture_get_required_blob_size(texture, false);
      align = 4096;

      MEM_FLAG_T mem_flag =
         MEM_FLAG_DIRECT | MEM_FLAG_DISCARDABLE | MEM_FLAG_RETAINED;
      if (secure)
         mem_flag |= (MEM_FLAG_SECURE | MEM_FLAG_NO_INIT);

      handle = mem_alloc_ex(size, align, mem_flag, "Texture blob", MEM_COMPACT_DISCARD);   // check, no term
      if (handle == MEM_INVALID_HANDLE) {
         texture->binding_type = TEXTURE_BLOB_NULL;
         return false;
      }

      vcos_assert(texture->current_item == 0);

      texture->blob_valid[buffer] |= 1<<level;
      texture->blob_pool[texture->current_item].secure = secure;
      assign_retain(texture_get_storage_handle_ref(texture), handle);
      assign_retain(&handle,MEM_INVALID_HANDLE);
      return true;
   }

   /*
      Second case: This fits into the already existing blob
   */
   if ((texture->binding_type == TEXTURE_STATE_UNDER_CONSTRUCTION || texture->binding_type == TEXTURE_STATE_COMPLETE_UNBOUND)
      && level < texture->blob_mip_count
      && width == _max(texture->width >> level, 1)
      && height == _max(texture->height >> level, 1)
      && khrn_image_to_tf_format(format) == khrn_image_to_tf_format(texture->format))
   {
      // Potentially somebody could be rendering into this image
      // So we need to flush now, otherwise they could overwrite whatever we're
      // about to put here.
      // TODO: this feels slightly inefficient
      // TODO: clean up the shared image case (i.e. when we need to recreate images and when we don't)

      //TODO this is where we'd do if write would block and if so, instantiate another item in the pool?
      texture_write_image_immediate(texture, buffer, level);

      texture->blob_valid[buffer] |= 1<<level;
      return true;
   }

   /*
      Remaining case: This does not fit with the other mipmaps. Create a new,
      separate image
   */

   /*
      In the dual core driver, makes sure any necessary cleanup happens before
      we start fiddling with texture memory handles
   */
   texture_write_image_immediate(texture, buffer, level);

   // Note that we allow the image to be wrong w.r.t. tformat/linear tile.
   // This means we won't regenerate the small mipmap levels of a pbuffer,
   // which are tformat.
   // All the same, if we're forced to create a new image we want to choose
   // the best format out of tformat and linear tile.

   if (khrn_image_prefer_lt(format, width, height))
      format = khrn_image_to_lt_format(format);

   himage = khrn_image_create(format, width, height,
      IMAGE_CREATE_FLAG_ONE | IMAGE_CREATE_FLAG_TEXTURE, secure); /* todo: check usage flags */
   if (himage == MEM_INVALID_HANDLE)
      return false;

   if (!invalidate_ms_image(texture, width, height, secure))
   {
      // image creation failed, so make the resolve invalid also
      mem_release(himage);
      return false;
   }

   if (texture->mh_mipmaps[buffer][level] == MEM_INVALID_HANDLE)
      texture->explicit_mipmaps++;

   MEM_ASSIGN(texture->mh_mipmaps[buffer][level], himage);
   mem_release(himage);

   texture->blob_valid[buffer] &= ~(1<<level);
   texture->binding_type = TEXTURE_STATE_UNDER_CONSTRUCTION;

   return true;
}

vcos_static_assert(GL_TEXTURE_CUBE_MAP_NEGATIVE_X == GL_TEXTURE_CUBE_MAP_POSITIVE_X + 1);
vcos_static_assert(GL_TEXTURE_CUBE_MAP_POSITIVE_Y == GL_TEXTURE_CUBE_MAP_NEGATIVE_X + 1);
vcos_static_assert(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y == GL_TEXTURE_CUBE_MAP_POSITIVE_Y + 1);
vcos_static_assert(GL_TEXTURE_CUBE_MAP_POSITIVE_Z == GL_TEXTURE_CUBE_MAP_NEGATIVE_Y + 1);
vcos_static_assert(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z == GL_TEXTURE_CUBE_MAP_POSITIVE_Z + 1);
static uint32_t convert_target(GLenum target)
{
   switch (target) {
   case GL_TEXTURE_2D:
      return TEXTURE_BUFFER_TWOD;
   case GL_TEXTURE_EXTERNAL_OES:
      return TEXTURE_BUFFER_EXTERNAL;
   case GL_TEXTURE_CUBE_MAP_POSITIVE_X:
   case GL_TEXTURE_CUBE_MAP_NEGATIVE_X:
   case GL_TEXTURE_CUBE_MAP_POSITIVE_Y:
   case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y:
   case GL_TEXTURE_CUBE_MAP_POSITIVE_Z:
   case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z:
      return TEXTURE_BUFFER_POSITIVE_X + target - GL_TEXTURE_CUBE_MAP_POSITIVE_X;
   default:
      UNREACHABLE();
      return 0;
   }
}

static bool update_mipmaps_and_blobs(GLXX_TEXTURE_T *texture, uint32_t level, uint32_t buffer)
{
   MEM_ASSIGN(texture->mh_depaletted_blob, MEM_INVALID_HANDLE);
   CHECK_BUFFER(buffer, texture);

   /*
      auto-generate mipmap if necessary
   */

   if (level == 0 && texture->generate_mipmap)
      return glxx_texture_generate_mipmap(texture, buffer, NULL);
   else
      return true;
}

static KHRN_IMAGE_FORMAT_T choose_copy_format(GLenum fmt, KHRN_IMAGE_FORMAT_T srcformat)
{
   // We must agree with fmt. We use 8-bit channels for most things, but for
   // 16-bit framebuffer formats we can carry on using that format - AS LONG AS
   // the requested internal format agrees. (So for RGBA5551->GL_RGB we need to
   // upsample, which is really horrible).
   // TODO: sort this mess out
   if (srcformat == RGBA_4444_TF && fmt == GL_RGBA)
   {
      return convert_format(fmt, GL_UNSIGNED_SHORT_4_4_4_4);
   }
   else if (srcformat == RGBA_5551_TF && fmt == GL_RGBA)
   {
      return convert_format(fmt, GL_UNSIGNED_SHORT_5_5_5_1);
   }
   else if (srcformat == RGB_565_TF && fmt == GL_RGB)
   {
      return convert_format(fmt, GL_UNSIGNED_SHORT_5_6_5);
   }
   else
   {
      return convert_format(fmt, GL_UNSIGNED_BYTE);
   }
}

static void mip_lock_wrap(GLXX_TEXTURE_T *texture, uint32_t buffer, uint32_t level, KHRN_IMAGE_WRAP_T *wrap)
{
   if (texture->blob_valid[buffer] & 1<<level) {
      khrn_image_interlock_wrap(wrap,
         glxx_texture_get_mipmap_format(texture, level, false),
         _max(texture->width >> level, 1),
         _max(texture->height >> level, 1),
         texture_get_mipmap_stride(texture, level, false),
         0,
         texture->blob_pool[texture->current_item].secure,
         (char *)mem_lock(glxx_texture_get_storage_handle(texture), NULL) + glxx_texture_get_mipmap_offset(texture, buffer, level, false), NULL);

      //this assert checks the blob is big enough for a tformat image, but we now also accept linear_tile images
      //vcos_assert(glxx_texture_get_mipmap_offset(texture, buffer, level, false) + get_size(wrap->format, wrap->width, wrap->height) <= mem_get_size(texture->mh_blob));
   } else {
      khrn_image_lock_interlock_wrap((KHRN_IMAGE_T *)mem_lock(texture->mh_mipmaps[buffer][level], NULL), wrap, NULL);
      mem_unlock(texture->mh_mipmaps[buffer][level]);
   }
}

static void mip_unlock_wrap(GLXX_TEXTURE_T *texture, uint32_t buffer, uint32_t level)
{
   if (texture->blob_valid[buffer] & 1<<level) {
      mem_unlock(glxx_texture_get_storage_handle(texture));
   } else {
      khrn_image_unlock_wrap((KHRN_IMAGE_T *)mem_lock(texture->mh_mipmaps[buffer][level], NULL));
      mem_unlock(texture->mh_mipmaps[buffer][level]);
   }
}

static bool has_mipmap(GLXX_TEXTURE_T *texture, uint32_t buffer, uint32_t level)
{
   if (texture->blob_valid[buffer] & 1<<level)
      return true;
   else if (texture->mh_mipmaps[buffer][level] != MEM_INVALID_HANDLE)
      return true;
   else
      return false;
}

static void null_image(GLXX_TEXTURE_T *texture, uint32_t buffer, uint32_t level)
{
   vcos_assert(texture->binding_type == TEXTURE_BLOB_NULL || texture->binding_type == TEXTURE_STATE_UNDER_CONSTRUCTION || texture->binding_type == TEXTURE_STATE_COMPLETE_UNBOUND);
   if (texture->binding_type == TEXTURE_STATE_COMPLETE_UNBOUND)
      texture->binding_type = TEXTURE_STATE_UNDER_CONSTRUCTION;

   if (texture->mh_mipmaps[buffer][level] != MEM_INVALID_HANDLE) {
      vcos_assert(texture->explicit_mipmaps);
      texture->explicit_mipmaps--;
      MEM_ASSIGN(texture->mh_mipmaps[buffer][level], MEM_INVALID_HANDLE);
   }
   texture->blob_valid[buffer] &= ~(1<<level);

   MEM_ASSIGN(texture->mh_ms_image, MEM_INVALID_HANDLE);
}

bool glxx_texture_image(GLXX_TEXTURE_T *texture, GLenum target, uint32_t level,
   uint32_t width, uint32_t height, GLenum fmt,
   GLenum type, uint32_t align, const void *pixels, bool secure)
{
   KHRN_IMAGE_FORMAT_T format, srcformat;
   uint32_t buffer;
   bool result;

   vcos_assert(texture);

   vcos_assert(level <= LOG2_MAX_TEXTURE_SIZE);

   format = convert_format(fmt, type);
   srcformat = convert_raster_format(fmt, type);

   buffer = convert_target(target);

   /* TODO: in the EGLImage case, stop this from copying the mipmap we'll be overwriting? */
   if (!texture_unbind(texture))
      return false;

   if (width == 0 || height == 0) {
      null_image(texture, buffer, level);
      return true;
   }

   /* For FBO to work in secure, we need a mechanism to tag them.  Unfortunatly, the API doesn't give us an
      indication.  If we generate a texture, but with no image data, I'm assuming its an FBO and not a CPU
      populated image.  We can always re-create in glTexSubImage() to handle the case it was guessed incorrectly */
   bool guess_secure = (secure && pixels == NULL);
   if (!check_image(texture, buffer, level, format, width, height, guess_secure))
      return false;

   if (pixels) {
      KHRN_IMAGE_WRAP_T dst_wrap, src_wrap;

      khrn_image_interlock_wrap(&src_wrap, srcformat,
         width, height, get_stride(width, fmt, type, align), 0, false, (void *)pixels, NULL);

      mip_lock_wrap(texture, buffer, level, &dst_wrap);
      if (!dst_wrap.secure)
         khrn_image_wrap_convert(&dst_wrap, &src_wrap, IMAGE_CONV_GL);
      mip_unlock_wrap(texture, buffer, level);
   }

   result = update_mipmaps_and_blobs(texture, level, buffer);
   //glxx_texture_check_complete(texture, false);
   return result;
}

bool glxx_texture_etc1_blank_image(GLXX_TEXTURE_T *texture, GLenum target, uint32_t level, uint32_t width, uint32_t height)
{
   uint32_t buffer;

   vcos_assert(texture);

   vcos_assert(level <= LOG2_MAX_TEXTURE_SIZE);

   buffer = convert_target(target);

   /* TODO: in the EGLImage case, stop this from copying the mipmap we'll be overwriting? */
   if (!texture_unbind(texture))
      return false;

   if (width == 0 || height == 0) {
      null_image(texture, buffer, level);
      return true;
   }

   if (!check_image(texture, buffer, level, ETC1_TF, width, height, false))
      return false;

   return update_mipmaps_and_blobs(texture, level, buffer);
}

void glxx_texture_etc1_sub_image(GLXX_TEXTURE_T *texture, GLenum target, uint32_t level, uint32_t dstx, uint32_t dsty, uint32_t width, uint32_t height, const void *data)
{
   KHRN_IMAGE_WRAP_T dst_wrap;
   uint32_t buffer;

   vcos_assert(texture);
   vcos_assert(data);
   vcos_assert(!(dstx & 3) && !(dsty & 3));

   vcos_assert(level <= LOG2_MAX_TEXTURE_SIZE);

   if (height == 0 || width == 0)
      return;

   buffer = convert_target(target);

   //mip_lock_wrap(texture, buffer, level, &dst_wrap); can we move this after the interlock check?

   // Potentially somebody could be rendering into this image
   // So we need to flush now, otherwise they could overwrite whatever we're
   // about to put here.
   texture_write_image_immediate(texture, buffer, level);

   mip_lock_wrap(texture, buffer, level, &dst_wrap);

   /* if the image is a multiple of the block size 4x4 */
   if (((dst_wrap.width & ~0x3) == dst_wrap.width) && ((dst_wrap.height & ~0x3) == dst_wrap.height))
   {
      KHRN_IMAGE_WRAP_T src_wrap;

      khrn_image_interlock_wrap(&src_wrap, ETC1, width, height, width / 2, 0, false, (void *)data, NULL);
      khrn_image_wrap_copy_region(&dst_wrap, dstx, dsty, width, height, &src_wrap, 0, 0, IMAGE_CONV_GL);
   }
   else
   {
      uint32_t w, h, x, y;

      w = (width + 3) / 4;
      h = (height + 3) / 4;
      for (y = 0; y < h; y++) {
         for (x = 0; x < w; x++) {
            uint32_t word0 = ((uint32_t*)data)[2 * (w * y + x)];
            uint32_t word1 = ((uint32_t*)data)[2 * (w * y + x) + 1];

            khrn_image_wrap_put_etc1_block(&dst_wrap, (dstx >> 2) + x, (dsty >> 2) + y, word0, word1);
         }
      }
   }

   mip_unlock_wrap(texture, buffer, level);
}

bool glxx_texture_copy_image(GLXX_TEXTURE_T *texture, GLenum target, uint32_t level, int width, int height, GLenum fmt, const KHRN_IMAGE_T *src, int srcx, int srcy)
{
   KHRN_IMAGE_WRAP_T mipmap_wrap, src_wrap;
   KHRN_IMAGE_FORMAT_T format;
   uint32_t buffer;
   uint32_t dstx = 0, dsty = 0;

   vcos_assert(texture);

   vcos_assert(level <= LOG2_MAX_TEXTURE_SIZE);

   // Choose the best internal representation
   format = choose_copy_format(fmt, src->format);

   buffer = convert_target(target);

   /* TODO: in the EGLImage case, stop this from copying the mipmap we'll be overwriting? */
   if (!texture_unbind(texture))
      return false;

   if (width == 0 || height == 0) {
      null_image(texture, buffer, level);
      return true;
   }

   vcos_demand(src->secure == false);
   if (!check_image(texture, buffer, level, format, width, height, false))
      return false;

   mip_lock_wrap(texture, buffer, level, &mipmap_wrap);

   if (srcx < 0) { dstx -= srcx; width += srcx;  srcx = 0; }
   if (srcy < 0) { dsty -= srcy; height += srcy; srcy = 0; }
   if (width > src->width   || srcx + width > src->width)   { width = src->width - srcx; }
   if (height > src->height || srcy + height > src->height) { height = src->height - srcy; }

   if (width > 0 && height > 0)
   {
      vcos_assert(srcx < srcx+width  && srcx+width  <= src->width);
      vcos_assert(srcy < srcy+height && srcy+height <= src->height);

      khrn_image_lock_interlock_wrap((KHRN_IMAGE_T *)src, &src_wrap, NULL);
      khrn_image_wrap_copy_region(&mipmap_wrap, dstx, dsty, width, height, &src_wrap, srcx, srcy, IMAGE_CONV_GL);
      khrn_image_unlock_wrap(src);
   }
   mip_unlock_wrap(texture, buffer, level);

   return update_mipmaps_and_blobs(texture, level, buffer);
}

bool glxx_texture_sub_image(GLXX_TEXTURE_T *texture, GLenum target, uint32_t level, uint32_t dstx, uint32_t dsty, uint32_t width, uint32_t height, GLenum fmt, GLenum type, uint32_t align, const void *pixels)
{
   KHRN_IMAGE_FORMAT_T srcformat;
   uint32_t buffer;
   KHRN_IMAGE_WRAP_T dst_wrap;

   vcos_assert(texture);

   vcos_assert(level <= LOG2_MAX_TEXTURE_SIZE);

   if (height == 0 || width == 0)
      return true;

   srcformat = convert_raster_format(fmt, type);

   buffer = convert_target(target);

   //mip_lock_wrap(texture, buffer, level, &dst_wrap); can we move this after the interlock check?

   // Potentially somebody could be rendering into this image
   // So we need to flush now, otherwise they could overwrite whatever we're
   // about to put here.
   texture_write_image_immediate(texture, buffer, level);

   mip_lock_wrap(texture, buffer, level, &dst_wrap);

   vcos_assert(dstx < dstx+width  && dstx+width  <= dst_wrap.width);
   vcos_assert(dsty < dsty+height && dsty+height <= dst_wrap.height);
   //TODO get KHRN_IMAGE_T to do this for us

   // Invalidate the ms image if it exists
   if (!invalidate_ms_image(texture, width, height, false))
   {
      // if the new ms image fails, don't copy the data and fail the api
      return false;
   }

   if (pixels) {
      KHRN_IMAGE_WRAP_T src_wrap;
      khrn_image_interlock_wrap(&src_wrap, srcformat, width, height, get_stride(width, fmt, type, align), 0, false, (void *)pixels, NULL);
      khrn_image_wrap_copy_region(
         &dst_wrap, dstx, dsty,
         width, height,
         &src_wrap, 0, 0,
         IMAGE_CONV_GL);
   }

   mip_unlock_wrap(texture, buffer, level);

   /*
      auto-generate mipmap if necessary
   */

   if (level == 0 && texture->generate_mipmap)
      return glxx_texture_generate_mipmap(texture, buffer, NULL);
   else
      return true;
}

bool glxx_texture_copy_sub_image(GLXX_TEXTURE_T *texture, GLenum target, uint32_t level, uint32_t dstx, uint32_t dsty, uint32_t width, uint32_t height, const KHRN_IMAGE_T *src, uint32_t srcx, uint32_t srcy)
{
   KHRN_IMAGE_WRAP_T dst_wrap, src_wrap;
   uint32_t buffer;

   vcos_assert(texture);
   vcos_assert(srcx < srcx+width  && srcx+width  <= src->width);
   vcos_assert(srcy < srcy+height && srcy+height <= src->height);

   vcos_assert(level <= LOG2_MAX_TEXTURE_SIZE);

   if (height == 0 || width == 0)
      return true;

   buffer = convert_target(target);

   // Potentially somebody could be rendering into this image
   // So we need to flush now, otherwise they could overwrite whatever we're
   // about to put here.
   texture_write_image_immediate(texture, buffer, level);

   mip_lock_wrap(texture, buffer, level, &dst_wrap);
   khrn_image_lock_interlock_wrap((KHRN_IMAGE_T *)src, &src_wrap, NULL);

   vcos_assert(dstx < dstx+width  && dstx+width  <= dst_wrap.width);
   vcos_assert(dsty < dsty+height && dsty+height <= dst_wrap.height);
   khrn_image_wrap_copy_region(&dst_wrap, dstx, dsty, width, height, &src_wrap, srcx, srcy, IMAGE_CONV_GL);

   khrn_image_unlock_wrap(src);
   mip_unlock_wrap(texture, buffer, level);

   /*
      auto-generate mipmap if necessary
   */

   if (level == 0 && texture->generate_mipmap)
      return glxx_texture_generate_mipmap(texture, buffer, NULL);
   else
      return true;
}

bool glxx_texture_paletted_blank_image(GLXX_TEXTURE_T *texture, GLenum target, uint32_t levels, uint32_t width, uint32_t height, KHRN_IMAGE_FORMAT_T format)
{
   uint32_t buffer;
   uint32_t w, h, level;

   vcos_assert(texture);
   vcos_assert(levels <= LOG2_MAX_TEXTURE_SIZE);

   buffer = convert_target(target);

   /* TODO: in the EGLImage case, stop this from copying the mipmap we'll be overwriting? */
   if (!texture_unbind(texture))
      return false;

   if (width == 0 || height == 0) {
      null_image(texture, buffer, 0);
      return true;
   }

   w = width;
   h = height;
   for (level = 0; level <= levels; level++)
   {
      if (!check_image(texture, buffer, level, format, w, h, false))
         return false;

      w = _max(w >> 1, 1);
      h = _max(h >> 1, 1);
   }

   return update_mipmaps_and_blobs(texture, ~0, buffer);   /* Set level to nonzero value to prevent mipmap regeneration */
}

void glxx_texture_paletted_sub_image(GLXX_TEXTURE_T *texture, GLenum target, uint32_t levels, uint32_t width, uint32_t height, const uint32_t *palette, uint32_t palsize, const void *data, int data_length)
{
   uint32_t buffer, x, y, w, h;
   int i = 0;
   const unsigned char *ptr = (const unsigned char*)data;
   bool firsthalf;
   uint32_t level;

   vcos_assert(texture);
   vcos_assert(data);

   vcos_assert(levels <= LOG2_MAX_TEXTURE_SIZE);

   if (height == 0 || width == 0)
      return;

   buffer = convert_target(target);

   firsthalf = true;
   w = width;
   h = height;
   for (level = 0; level <= levels; level++) {
      KHRN_IMAGE_WRAP_T dst_wrap;

      mip_lock_wrap(texture, buffer, level, &dst_wrap);
      for (y = 0; y < h; y++) {
         for (x = 0; x < w; x++) {
            unsigned char idx;
            uint32_t pixel;

            if (i >= 0 && i < data_length) {
               if (palsize == 256)
                  idx = ptr[i];
               else if (firsthalf)
                  idx = ptr[i] >> 4;
               else
                  idx = ptr[i] & 15;

               pixel = palette[idx];

               khrn_image_wrap_put_pixel(&dst_wrap, x, y, pixel);
            }

            if (palsize != 256)
               firsthalf = !firsthalf;

            if (firsthalf)
               i++;
         }
      }

      w = _max(w >> 1, 1);
      h = _max(h >> 1, 1);
      mip_unlock_wrap(texture, buffer, level);
   }
}

bool glxx_texture_generate_mipmap(GLXX_TEXTURE_T *texture, uint32_t buffer, bool *invalid_operation)
{
   /*
    * TODO: this may be a spec deviation. What happens if:
    * - we create a pbuffer without mipmaps
    * - we bind this pbuffer to a texture
    * - we call glGenerateMipmap
    * At present, glGenerateMipmap does nothing. (This behaviour seems the most
    * consistent with the auto mipmap generation behaviour specified in EGL).
    *
    * We set invalid_operation to true if we were unable to complete but didn't run
    * out of memory. It is assumed that automatic mipmap generation doesn't care about
    * this, and that glGenerateMipmaps will set GL_INVALID_OPERATION.
    */

   if (invalid_operation)
      *invalid_operation = true;

   if (texture->mipmaps_available && has_mipmap(texture, buffer, 0))
   {
      KHRN_IMAGE_WRAP_T wrap0;
      KHRN_IMAGE_FORMAT_T format;
      uint32_t width, height, level;

      vcos_assert(buffer <= TEXTURE_BUFFER_NEGATIVE_Z);
      mip_lock_wrap(texture, buffer, 0, &wrap0);

      width = wrap0.width >> 1;
      height = wrap0.height >> 1;

      level = 1;

      format = khrn_image_to_tf_format(wrap0.format);

      mip_unlock_wrap(texture, buffer, 0);

      /* Don't attempt to generate mipmaps for ETC1 textures */
      if (khrn_image_is_color(format))
      {
         if (invalid_operation)
            *invalid_operation = false;

         while (width > 0 || height > 0)
         {
            KHRN_IMAGE_WRAP_T src_wrap, dst_wrap;

            width = _max(width, 1);
            height = _max(height, 1);

            if (texture->binding_type != TEXTURE_STATE_BOUND_TEXIMAGE)
            {
               if (!check_image(texture, buffer, level, format, width, height, false))
                  return false;
            }

            mip_lock_wrap(texture, buffer, level - 1, &src_wrap);
            mip_lock_wrap(texture, buffer, level, &dst_wrap);
            khrn_image_wrap_subsample(&dst_wrap, &src_wrap);
            mip_unlock_wrap(texture, buffer, level);
            mip_unlock_wrap(texture, buffer, level - 1);

            width >>= 1;
            height >>= 1;

            level++;
         }
      }
   }

   return true;
}

static bool keep_explicit_mipmaps(GLXX_TEXTURE_T *texture)
{
   /*
      If:
      - this texture is being used as a framebuffer (or might be), or
      - it has been created from an EGL surface, or
      - it is an EGL image sibling

      i.e. if it shares any image data with other objects, then we need to hold
      onto any mipmap images we've created, in case they're shared with
      something.
   */
   return texture->framebuffer_sharing
      || texture->binding_type == TEXTURE_STATE_BOUND_TEXIMAGE
      || texture->binding_type == TEXTURE_STATE_BOUND_EGLIMAGE;
}

static void buffer_minmax(const GLXX_TEXTURE_T *texture,
        uint32_t *min_buffer, uint32_t *max_buffer)
{
   switch (texture->target) {
   case GL_TEXTURE_2D:
      *min_buffer = TEXTURE_BUFFER_TWOD;
      *max_buffer = TEXTURE_BUFFER_TWOD;
      break;
   case GL_TEXTURE_EXTERNAL_OES:
      *min_buffer = TEXTURE_BUFFER_EXTERNAL;
      *max_buffer = TEXTURE_BUFFER_EXTERNAL;
      break;
   case GL_TEXTURE_CUBE_MAP:
      *min_buffer = TEXTURE_BUFFER_POSITIVE_X;
      *max_buffer = TEXTURE_BUFFER_NEGATIVE_Z;
      break;
   default:
      UNREACHABLE();
      *min_buffer = *max_buffer = 0;
   }
}

static GLXX_TEXTURE_COMPLETENESS_T copy_mipmaps_to_new_blob(GLXX_TEXTURE_T *texture)
{
   bool complete = true;
   MEM_HANDLE_T handle;
   KHRN_IMAGE_WRAP_T src_wrap, dst_wrap;
   GLXX_TEXTURE_T tdummy;
   uint32_t min_buffer, max_buffer, size, align, buffer, level, mipmap_count;

   buffer_minmax(texture, &min_buffer, &max_buffer);

   if (!has_mipmap(texture, min_buffer, 0))
      return INCOMPLETE;

   mip_lock_wrap(texture, min_buffer, 0, &src_wrap);

   /* TODO this is hacky */
   tdummy.width = src_wrap.width;
   tdummy.height = src_wrap.height;
   tdummy.format = convert_to_lt_format(src_wrap.format);
   tdummy.target = texture->target;
   tdummy.min = texture->min;
   tdummy.current_item = 0;
   tdummy.blob_pool[0].mh_storage = MEM_INVALID_HANDLE;
   tdummy.binding_type = TEXTURE_STATE_UNDER_CONSTRUCTION;
   tdummy.blob_mip_count = glxx_texture_get_mipmap_count(&tdummy);
   tdummy.offset = 0;  /* needed for correct texture_get_required_blob_size */
   tdummy.mip0_offset = ~0;

   mip_unlock_wrap(texture, min_buffer, 0);

   size = texture_get_required_blob_size(&tdummy, false);
   align = 4096;
   handle = mem_alloc_ex(size, align, MEM_FLAG_DIRECT | MEM_FLAG_DISCARDABLE | MEM_FLAG_RETAINED, "Texture blob", MEM_COMPACT_DISCARD);   // check, no term

   if (handle == MEM_INVALID_HANDLE)
      return OUT_OF_MEMORY;

   MEM_ASSIGN(tdummy.blob_pool[0].mh_storage, handle);
   memset(tdummy.blob_valid, 0, sizeof(tdummy.blob_valid));

   mipmap_count = tdummy.blob_mip_count;
   for (buffer = min_buffer; buffer <= max_buffer; buffer++) {
      for (level = 0; level < mipmap_count; level++) {
         if (has_mipmap(texture, buffer, level)) {
            tdummy.blob_valid[buffer] |= 1<<level;

            mip_lock_wrap(&tdummy, buffer, level, &dst_wrap);
            mip_lock_wrap(texture, buffer, level, &src_wrap);

            if (dst_wrap.width == src_wrap.width && dst_wrap.height == src_wrap.height
                && convert_to_lt_format(dst_wrap.format) == convert_to_lt_format(src_wrap.format) ) {
               khrn_image_wrap_convert(&dst_wrap, &src_wrap, IMAGE_CONV_GL);
               mip_unlock_wrap(texture, buffer, level);

               if (texture->mh_mipmaps[buffer][level] != MEM_INVALID_HANDLE) {
                  if (keep_explicit_mipmaps(texture)) {
                     /*
                        We have been told to keep this image, but need to point it
                        towards the new blob.
                     */
                     KHRN_IMAGE_T *image = (KHRN_IMAGE_T *)mem_lock(texture->mh_mipmaps[buffer][level], NULL);

                     MEM_ASSIGN(image->mh_storage, handle);
                     image->offset = glxx_texture_get_mipmap_offset(&tdummy, buffer, level, false);

                     mem_unlock(texture->mh_mipmaps[buffer][level]);
                  } else {
                     MEM_ASSIGN(texture->mh_mipmaps[buffer][level], MEM_INVALID_HANDLE);
                     vcos_assert(texture->explicit_mipmaps);
                     texture->explicit_mipmaps--;
                  }
               }
               texture->blob_valid[buffer] |= 1<<level;
            } else {
               complete = false;
               mip_unlock_wrap(texture, buffer, level);
               texture->blob_valid[buffer] &= ~(1<<level);
            }

            mip_unlock_wrap(&tdummy, buffer, level);
         } else
            complete = false;
      }
      texture->blob_valid[buffer] &= (1<<mipmap_count) - 1;
   }

   texture->width = tdummy.width;
   texture->height = tdummy.height;
   texture->format = tdummy.format;
   texture->blob_mip_count = tdummy.blob_mip_count;
   assign_retain(&texture->blob_pool[texture->current_item].mh_storage, handle);
   texture->offset = 0;
   texture->binding_type = complete ? TEXTURE_STATE_COMPLETE_UNBOUND : TEXTURE_STATE_UNDER_CONSTRUCTION;
   texture->mip0_offset = ~0;

   mem_release(tdummy.blob_pool[0].mh_storage);
   assign_retain(&handle, MEM_INVALID_HANDLE);

   return complete ? COMPLETE : INCOMPLETE;
}

static GLXX_TEXTURE_COMPLETENESS_T copy_mipmaps_to_current_blob(GLXX_TEXTURE_T *texture)
{
   bool complete = true;
   KHRN_IMAGE_WRAP_T src_wrap, dst_wrap;
   uint32_t min_buffer, max_buffer, buffer, level, mipmap_count;

   vcos_assert(glxx_texture_get_storage_handle(texture) != MEM_INVALID_HANDLE);

   buffer_minmax(texture, &min_buffer, &max_buffer);

   vcos_assert(texture->blob_mip_count >= glxx_texture_get_mipmap_count(texture));
   mipmap_count = glxx_texture_get_mipmap_count(texture);
   for (buffer = min_buffer; buffer <= max_buffer; buffer++) {
      for (level = 0; level < mipmap_count; level++) {
         if (!(texture->blob_valid[buffer] & 1<<level)) {
            if (texture->mh_mipmaps[buffer][level] != MEM_INVALID_HANDLE) {
               bool ok = false;
               if (texture->blob_valid[buffer] & 1<<level) {
#ifndef NDEBUG
                  KHRN_IMAGE_T *src;
#endif
                  /*
                     Image is already installed in the right place.

                     (If we are told to keep explicit mipmaps then we can leave
                     this image exactly as it is).
                  */
                  ok = true;
                  vcos_assert((src = (KHRN_IMAGE_T *)mem_lock(texture->mh_mipmaps[buffer][level], NULL), 1));
                  vcos_assert(src->mh_storage == glxx_texture_get_storage_handle(texture));
                  vcos_assert(src->offset == glxx_texture_get_mipmap_offset(texture, buffer, level, false));
                  vcos_assert((mem_unlock(texture->mh_mipmaps[buffer][level]),1));
               } else {
                  KHRN_IMAGE_T *src = (KHRN_IMAGE_T *)mem_lock(texture->mh_mipmaps[buffer][level], NULL);

                  texture->blob_valid[buffer] |= 1<<level;  /* need to do this early for mip_lock_wrap to work */

                  mip_lock_wrap(texture, buffer, level, &dst_wrap);

                  if (dst_wrap.width == src->width && dst_wrap.height == src->height && khrn_image_to_tf_format(dst_wrap.format) == khrn_image_to_tf_format(src->format)) {
                     khrn_image_lock_interlock_wrap(src, &src_wrap, NULL);
                     khrn_image_wrap_convert(&dst_wrap, &src_wrap, IMAGE_CONV_GL);
                     khrn_image_unlock_wrap(src);
                     ok = true;

                     if (keep_explicit_mipmaps(texture)) {
                        /*
                           We have been told to keep this image, but need to point it
                           towards the blob.

                           Note we make sure the image data isn't locked at this point.
                        */
                        MEM_ASSIGN(src->mh_storage, glxx_texture_get_storage_handle(texture));
                        src->offset = glxx_texture_get_mipmap_offset(texture, buffer, level, false);
                     }
                     mip_unlock_wrap(texture, buffer, level);
                  } else {
                     mip_unlock_wrap(texture, buffer, level);
                     complete = false;
                     texture->blob_valid[buffer] &= ~(1<<level);
                  }
                  mem_unlock(texture->mh_mipmaps[buffer][level]);
               }

               if (ok && !keep_explicit_mipmaps(texture)) {
                  /*
                     We need to keep hold of this image if:
                     - we're told to (keep_explicit_mipmaps), or
                     - the image isn't the right shape/format for our blob (!ok)

                     Note we make sure we've finished with the image before deleting it.
                  */
                  MEM_ASSIGN(texture->mh_mipmaps[buffer][level], MEM_INVALID_HANDLE);
                  vcos_assert(texture->explicit_mipmaps);
                  texture->explicit_mipmaps--;
               }
            } else {
               /*
                  Missing mipmap implies texture is not complete.
               */
               complete = false;
               break;
            }
         }
      }
   }

   if (complete)
   {
      vcos_assert(texture->binding_type == TEXTURE_BLOB_NULL || texture->binding_type == TEXTURE_STATE_UNDER_CONSTRUCTION);
      texture->binding_type = TEXTURE_STATE_COMPLETE_UNBOUND;
   }

   return complete ? COMPLETE : INCOMPLETE;
}

bool glxx_texture_is_cube_complete(GLXX_TEXTURE_T *texture)
{
   int buffer;
   KHRN_IMAGE_WRAP_T px_wrap;

   for (buffer = TEXTURE_BUFFER_POSITIVE_X; buffer <= TEXTURE_BUFFER_NEGATIVE_Z; buffer++)
      if (!has_mipmap(texture, buffer, 0))
         return false;

   mip_lock_wrap(texture, TEXTURE_BUFFER_POSITIVE_X, 0, &px_wrap);
   mip_unlock_wrap(texture, TEXTURE_BUFFER_POSITIVE_X, 0);

   /* This feels dirty, but the non-storage part of the wrap structure will still be valid... */

   vcos_assert(px_wrap.width == px_wrap.height);  /* shouldn't be possible to install non-square images */

   for (buffer = TEXTURE_BUFFER_NEGATIVE_X; buffer <= TEXTURE_BUFFER_NEGATIVE_Z; buffer++)
   {
      KHRN_IMAGE_WRAP_T wrap;

      mip_lock_wrap(texture, buffer, 0, &wrap);
      mip_unlock_wrap(texture, buffer, 0);

      if (wrap.width != px_wrap.width || wrap.height != px_wrap.height || wrap.format != px_wrap.format)
         return false;
   }

   return true;
}

// TODO: this comment is out of date
// This does some fancy stuff (e.g. generating the blob) but is guaranteed not
// to call khrn_sharebuffer_flush_buffer. This means you can install a framebuffer
// before calling this function and it will still be there at the end.

GLXX_TEXTURE_COMPLETENESS_T glxx_texture_check_complete(GLXX_TEXTURE_T *texture)
{
   //TODO: internal formats have same colour components but different bit depths
   uint32_t min_buffer, max_buffer;
   GLXX_TEXTURE_COMPLETENESS_T result = INCOMPLETE;

   buffer_minmax(texture, &min_buffer, &max_buffer);

   if (texture->binding_type == TEXTURE_STATE_BOUND_TEXIMAGE || texture->binding_type == TEXTURE_STATE_BOUND_EGLIMAGE)
      result = COMPLETE;   /* Assume that bound textures are always complete */
   else if (texture->binding_type == TEXTURE_BLOB_NULL || !(texture->blob_valid[min_buffer] & 1<<0) || texture->blob_mip_count < glxx_texture_get_mipmap_count(texture))
      result = copy_mipmaps_to_new_blob(texture);
   else if (texture->binding_type == TEXTURE_STATE_UNDER_CONSTRUCTION)
      result = copy_mipmaps_to_current_blob(texture);
   else if (texture->blob_valid[min_buffer] & 1<<0)
      result = COMPLETE;   /* texture already complete and blobbed, and has all the mipmaps we need */

   return result;
}

void glxx_texture_bind_images(GLXX_TEXTURE_T *texture, uint32_t levels, MEM_HANDLE_T *images, uint32_t binding_type, MEM_HANDLE_T bound_data, int mipmap_level)
{
   uint32_t i;
   uint32_t buf = 0;

   UNUSED(bound_data);

   vcos_assert(levels <= LOG2_MAX_TEXTURE_SIZE);
   vcos_assert(binding_type == TEXTURE_STATE_BOUND_TEXIMAGE || binding_type == TEXTURE_STATE_BOUND_EGLIMAGE);

   buf = convert_target(texture->target);

   glxx_texture_release_teximage(texture);
   nullify(texture);

   texture->binding_type = binding_type;
   texture->blob_mip_count = levels;
   for (i = 0; i < levels; i++) {
      KHRN_IMAGE_T *image;
#ifndef NDEBUG
      KHRN_IMAGE_WRAP_T blob_wrap;
#endif

      vcos_assert(images[i] != MEM_INVALID_HANDLE);
      vcos_assert(texture->mh_mipmaps[buf][i] == MEM_INVALID_HANDLE);
      MEM_ASSIGN(texture->mh_mipmaps[buf][i], images[i]);
      texture->explicit_mipmaps++;

      image = (KHRN_IMAGE_T *)mem_lock(images[i], NULL);

      if (i == 0) {
         /* Base our texture on this image and assume that all subsequent images fit */
         texture->width = image->width;
         texture->height = image->height;
         /* raster images can be supported in 2 ways:
            1. a limited number of pot sized raster formats are supported by the V3D hardware
            2. OES_draw_texture has a fast path that uses the bcm composition library not the V3D

            at present this code supports #2 only - TODO add a flag to clearly identify this path
         */
         vcos_assert(khrn_image_is_tformat(image->format) || khrn_image_is_lineartile(image->format) || khrn_image_is_rso(image->format));
         texture->format = khrn_image_is_rso(image->format) ? image->format : khrn_image_to_tf_format(image->format); /*_TF: mipmaps are all t-format */
         assign_retain(texture_get_storage_handle_ref(texture), image->mh_storage);
         texture->mip0_offset = ~0;
         texture->offset = image->offset;
      }

      texture->blob_valid[buf] |= 1<<i;

      if (binding_type == TEXTURE_STATE_BOUND_TEXIMAGE) {

#ifndef NDEBUG
         /* Check that image agrees completely with what it should be */
         mip_lock_wrap(texture, buf, i, &blob_wrap);   /* This can't be called until binding_type, dimensions, etc. are set */
         vcos_assert(image->format == blob_wrap.format);
         vcos_assert(image->width == blob_wrap.width);
         vcos_assert(image->height == blob_wrap.height);
         vcos_assert(image->stride == blob_wrap.stride);
         vcos_assert(image->mh_storage == glxx_texture_get_storage_handle(texture));
         vcos_assert(image->offset == glxx_texture_get_mipmap_offset(texture, buf, i, false));
         mip_unlock_wrap(texture, buf, i);
#endif

         vcos_assert(!(image->flags & IMAGE_FLAG_BOUND_TEXIMAGE));  /* we set this flag */
         image->flags |= IMAGE_FLAG_BOUND_TEXIMAGE;
      } else {
         vcos_assert(image->flags & IMAGE_FLAG_BOUND_EGLIMAGE);     /* EGL sets this flag */
         /* remove the IMAGE_FLAG_DISPLAY so any copytext is just mapped as a one to one copy (not inverted) */
         /* JSH -> If the user mapped this mapped texture to an FBO, rendered into it and then read the data back from the
            data pointer, then the image would be upside down. */
         image->flags &= ~IMAGE_FLAG_DISPLAY;
      }
      mem_unlock(images[i]);
   }

   /*
   "Texture mipmap levels are automatically generated when all of the following
   conditions are met while calling eglBindTexImage:
   - The EGL MIPMAP TEXTURE attribute of the pbuffer being bound is
   EGL_TRUE.
   - The OpenGL ES texture parameter GL_GENERATE_MIPMAP is GL_TRUE for
   the currently bound texture.
   - The value of the EGL_MIPMAP_LEVEL attribute of the pbuffer being bound is
   equal to the value of the texture parameter GL_TEXTURE_BASE_LEVEL."
   */
   texture->mipmaps_available = levels > 1;

   /*
      should always succeed, as we've just supplied an appropriate list of images
      to keep check_image happy
   */
   verify(update_mipmaps_and_blobs(texture, mipmap_level, buf));
}

/*
   Khronos documentation:
   [EGL]
   It is not an error to call glTexImage2D or glCopyTexImage2D to replace an
   image of a texture object that has a color buffer bound to it. However, these calls
   will cause the color buffer to be released back to the surface and new memory will
   be allocated for the texture. Note that the color buffer is released even if the image
   that is being defined is a mipmap level that was not defined by the color buffer.

   [OES_egl_image]
    If an application later respecifies any image
    array in the texture object (through mechanisms such as calls to
    TexImage2D and/or GenerateMipmapOES, or setting the
    SGIS_GENERATE_MIPMAP parameter to TRUE), implementations should allocate
    additional space for all specified (and respecified) image arrays,
    and copy any existing image data to the newly (re)specified texture
    object (as if TexImage was called for every level-of-detail in the
    texture object).  The respecified texture object will not be an
    EGLImage target.
*/

static bool texture_unbind(GLXX_TEXTURE_T *texture)
{
   uint32_t i;
   uint32_t buf = 0;
   switch (texture->binding_type) {
   case TEXTURE_BLOB_NULL:
   case TEXTURE_STATE_UNDER_CONSTRUCTION:
   case TEXTURE_STATE_COMPLETE_UNBOUND:
      return true;

   case TEXTURE_STATE_BOUND_TEXIMAGE:
      /* Unbinding causes texture to be nullified */
      if (texture->target == GL_TEXTURE_CUBE_MAP) {
         int j;
         for (j = TEXTURE_BUFFER_POSITIVE_X; j <= TEXTURE_BUFFER_NEGATIVE_Z; j++) {
            for (i = 0; i < LOG2_MAX_TEXTURE_SIZE + 1; i++) {
               if (texture->mh_mipmaps[j][i] != MEM_INVALID_HANDLE) {
                  ((KHRN_IMAGE_T *)mem_lock(texture->mh_mipmaps[j][i], NULL))->flags &= ~IMAGE_FLAG_BOUND_TEXIMAGE;  /* we clear this flag */
                  mem_unlock(texture->mh_mipmaps[j][i]);
               }
            }
         }
      } else {
         buf = convert_target(texture->target);
         for (i = 0; i < LOG2_MAX_TEXTURE_SIZE + 1; i++) {
            if (texture->mh_mipmaps[buf][i] != MEM_INVALID_HANDLE) {
               ((KHRN_IMAGE_T *)mem_lock(texture->mh_mipmaps[buf][i], NULL))->flags &= ~IMAGE_FLAG_BOUND_TEXIMAGE;  /* we clear this flag */
               mem_unlock(texture->mh_mipmaps[buf][i]);
            }
         }
      }
      nullify(texture);
      texture->binding_type = 0;
      return true;

   case TEXTURE_STATE_BOUND_EGLIMAGE:  /* TODO: does this work for EGLImage sources as well as targets? */
   {
      vcos_assert(texture->target != GL_TEXTURE_CUBE_MAP);

      /* Set these now so that copy_mipmaps_to_new_blob is not confused by keep_explicit_mipmaps */
      texture->binding_type = TEXTURE_STATE_UNDER_CONSTRUCTION;  /* Should be complete, but this feels safer... */
      texture->framebuffer_sharing = false;
      MEM_ASSIGN(texture->external_image, MEM_INVALID_HANDLE);

      return copy_mipmaps_to_new_blob(texture) != OUT_OF_MEMORY;
   }
   default:
      UNREACHABLE();
      return false;
   }
}

void glxx_texture_release_teximage(GLXX_TEXTURE_T *texture)
{
   if (texture->binding_type == TEXTURE_STATE_BOUND_TEXIMAGE)
      verify(texture_unbind(texture));
}

uint32_t glxx_texture_get_mipmap_count(GLXX_TEXTURE_T *texture)
{
   switch (texture->min) {
   case GL_NEAREST:
   case GL_LINEAR:
      return 1;
   case GL_NEAREST_MIPMAP_NEAREST:
   case GL_NEAREST_MIPMAP_LINEAR:
   case GL_LINEAR_MIPMAP_NEAREST:
   case GL_LINEAR_MIPMAP_LINEAR:
   {
      int w = texture->width;
      int h = texture->height;
      uint32_t result;

      vcos_assert(w > 0 && h > 0);
      result = _msb(_max(w, h)) + 1;
      vcos_assert(result >= 1 && result <= LOG2_MAX_TEXTURE_SIZE + 1);
      return result;
   }
   default:
      UNREACHABLE();
      return 1;
   }
}

static uint32_t texture_get_required_blob_size(GLXX_TEXTURE_T *texture, bool depaletted)
{
   uint32_t res = 0;

   switch (texture->target) {
   case GL_TEXTURE_2D:
   case GL_TEXTURE_EXTERNAL_OES:   /* Exactly the same size as 2D */
      res = glxx_texture_get_mipmap_offset(texture,
               TEXTURE_BUFFER_TWOD, 0, depaletted)
         + texture_get_mipmap_size(texture, 0, depaletted);
      break;
   case GL_TEXTURE_CUBE_MAP:
      res = glxx_texture_get_mipmap_offset(texture,
               TEXTURE_BUFFER_NEGATIVE_Z+1,
               glxx_texture_get_mipmap_count(texture)-1, depaletted);
      break;
   default:
      break;
   }

   return res;
}

uint32_t glxx_texture_get_cube_stride(GLXX_TEXTURE_T *texture)
{
   uint32_t cube_stride = 0;
   //uint32_t mipmap_count = glxx_texture_get_mipmap_count(texture);
   uint32_t mipmap_count = texture->blob_mip_count;
   uint32_t i;

   vcos_assert(texture->target == GL_TEXTURE_CUBE_MAP);

   for (i = 0; i < mipmap_count; i++) {
      cube_stride += texture_get_mipmap_size(texture, i, false);
   }
   cube_stride = (cube_stride + 4095) & ~4095;

   return cube_stride;
}

uint32_t glxx_texture_get_mipmap_offset(GLXX_TEXTURE_T *texture, uint32_t buffer, uint32_t level, bool depaletted)
{
   uint32_t result = texture->offset, i;
   uint32_t mipmap_count;
   uint32_t result0;
   bool cacheable = ((level == 0) &&
       (buffer == TEXTURE_BUFFER_TWOD || buffer == TEXTURE_BUFFER_EXTERNAL));

   vcos_assert(level <= LOG2_MAX_TEXTURE_SIZE+1);
   vcos_assert(!depaletted || texture->mh_depaletted_blob != MEM_INVALID_HANDLE);

   if (cacheable && texture->mip0_offset != (uint32_t)~0)
      return texture->mip0_offset;

   mipmap_count = texture->blob_mip_count;
   vcos_assert(mipmap_count != 0);
   result0 = texture->offset;
   for (i = mipmap_count - 1; i > level; i--) {
      result += texture_get_mipmap_size(texture, i, depaletted);
   }
   /* TODO: should probably use texture->offset for this alignment hack */
   for (i = mipmap_count - 1; i > 0; i--) {
      result0 += texture_get_mipmap_size(texture, i, depaletted);
   }
   result += (-(int32_t)result0) & 0xfff;

   if (cacheable)
      texture->mip0_offset = result;

   if (texture->target == GL_TEXTURE_CUBE_MAP) {
      result += glxx_texture_get_cube_stride(texture) *
         (buffer - TEXTURE_BUFFER_POSITIVE_X);
   }

   return result;
}

static uint32_t get_size(KHRN_IMAGE_FORMAT_T format, uint32_t width, uint32_t height)
{
   return khrn_image_get_stride(format, width) * khrn_image_pad_height(format, height);
}

static uint32_t texture_get_mipmap_size(GLXX_TEXTURE_T *texture, uint32_t level, bool depaletted)
{
   KHRN_IMAGE_FORMAT_T format;
   uint32_t result;
   uint32_t width, height;

   vcos_assert(level <= LOG2_MAX_TEXTURE_SIZE);
   vcos_assert(!depaletted || texture->mh_depaletted_blob != MEM_INVALID_HANDLE);
   vcos_assert(texture->binding_type != TEXTURE_BLOB_NULL);

   format = glxx_texture_get_mipmap_format(texture, level, depaletted);
   width = texture->width;
   height = texture->height;
   if (level > 0)
   {
      width = 1 << _msb(2*width-1);
      height = 1 << _msb(2*height-1);
   }
   width = _max(width >> level, 1);
   height = _max(height >> level, 1);
   result = get_size(format, width, height);
   return result;
}

static uint32_t texture_get_mipmap_stride(GLXX_TEXTURE_T *texture, uint32_t level, bool depaletted)
{
   KHRN_IMAGE_FORMAT_T format;
   uint32_t result;
   uint32_t width;

   vcos_assert(level <= LOG2_MAX_TEXTURE_SIZE);
   vcos_assert(!depaletted || texture->mh_depaletted_blob != MEM_INVALID_HANDLE);
   vcos_assert(texture->binding_type != TEXTURE_BLOB_NULL);

   format = glxx_texture_get_mipmap_format(texture, level, depaletted);
   width = texture->width;
   if (level > 0)
   {
      width = 1 << _msb(2*width-1);
   }
   width = _max(width >> level, 1);
   result = khrn_image_get_stride(format, width);

   return result;
}

KHRN_IMAGE_FORMAT_T glxx_texture_get_mipmap_format(GLXX_TEXTURE_T *texture, uint32_t level, bool depaletted)
{
   KHRN_IMAGE_FORMAT_T result;
   uint32_t width, height;
   //uint32_t utw = khrn_image_get_log2_utile_width(texture->format);
   //uint32_t w = (texture->width+(1<<utw)-1)>>utw;
   //uint32_t uth = khrn_image_get_log2_utile_height(texture->format);
   //uint32_t h = (texture->height+(1<<uth)-1)>>uth;
   //bool all_tformat = khrn_image_is_tformat(texture->format);
   bool prefer_ltformat;

   width = texture->width;
   height = texture->height;
   if (level > 0)
   {
      width = 1 << _msb(2*width-1);
      height = 1 << _msb(2*height-1);
   }
   width = _max(width >> level, 1);
   height = _max(height >> level, 1);
   prefer_ltformat = khrn_image_is_rso(texture->format) ? 0 : khrn_image_prefer_lt(texture->format, width, height);

   //w = _max(w >> level, 1);
   //h = _max(h >> level, 1);

   vcos_assert(level <= LOG2_MAX_TEXTURE_SIZE);
   vcos_assert(!depaletted || texture->mh_depaletted_blob != MEM_INVALID_HANDLE);
   vcos_assert(texture->binding_type != TEXTURE_BLOB_NULL);

   if (depaletted) {
      //if (w >= 8 && h >= 8 || all_tformat)
      if (!prefer_ltformat)
         result = ABGR_8888_TF;
      else
         result = ABGR_8888_LT;
   } else {
      //if (w >= 8 && h >= 8 || all_tformat)
      if (!prefer_ltformat)
         result = khrn_image_is_rso(texture->format) ? texture->format : khrn_image_to_tf_format(texture->format);
      else
         result = khrn_image_to_lt_format(texture->format);
   }

   return result;
}

KHRN_IMAGE_FORMAT_T glxx_texture_incomplete_get_mipmap_format(GLXX_TEXTURE_T *texture, GLenum target, uint32_t level)
{
   uint32_t buffer = convert_target(target);

   vcos_assert(level <= LOG2_MAX_TEXTURE_SIZE);
   if (texture->blob_valid[buffer] & (1 << level))
      return glxx_texture_get_mipmap_format(texture, level, false);
   else if (texture->mh_mipmaps[buffer][level] == MEM_INVALID_HANDLE)
      return IMAGE_FORMAT_INVALID;
   else
   {
      KHRN_IMAGE_T *img;
      KHRN_IMAGE_FORMAT_T result;

      img = (KHRN_IMAGE_T *)mem_lock(texture->mh_mipmaps[buffer][level], NULL);
      result = img->format;
      mem_unlock(texture->mh_mipmaps[buffer][level]);

      return result;
   }
}

// All mipmap levels should be t-format or linear tile versions of the same format
// This returns it converted to t-format
KHRN_IMAGE_FORMAT_T glxx_texture_get_tformat(GLXX_TEXTURE_T *texture)
{
   KHRN_IMAGE_FORMAT_T format;
   format = khrn_image_is_rso(texture->format) ? texture->format : khrn_image_to_tf_format(texture->format);
   format = khrn_image_no_colorspace_format(format);
   return format;
}

bool glxx_texture_includes(GLXX_TEXTURE_T *texture, GLenum target, int level, int x, int y)
{
   uint32_t buffer = convert_target(target);
   uint32_t width = 0, height = 0;

   CHECK_BUFFER(buffer, texture);

   if (level < 0 || level >= LOG2_MAX_TEXTURE_SIZE + 1)
      return false;

   if (x < 0 || y < 0)
      return false;

   if ((texture->blob_valid[buffer] & 1<<level)) {
      width = _max(texture->width >> level, 1);
      height = _max(texture->height >> level, 1);
   } else if (texture->mh_mipmaps[buffer][level] != MEM_INVALID_HANDLE) {
      KHRN_IMAGE_T *image = (KHRN_IMAGE_T *)mem_lock(texture->mh_mipmaps[buffer][level], NULL);
      width = image->width;
      height = image->height;
      mem_unlock(texture->mh_mipmaps[buffer][level]);
   } else
      return false;

   return (uint32_t)x < width && (uint32_t)y < height;
}

bool glxx_texture_is_valid_image(KHRN_IMAGE_T *image)
{
   KHRN_IMAGE_FORMAT_T format;

   vcos_assert(image);

   if (image->width > MAX_TEXTURE_SIZE || image->height > MAX_TEXTURE_SIZE)
      return false;

   /* for the fast path of OES_draw_textures and for ABGR we support raster order formats */
   if (!(khrn_image_is_tformat(image->format) || khrn_image_is_lineartile(image->format) || khrn_image_is_rso(image->format)))
      return false;

   if(khrn_image_is_tformat(image->format) || khrn_image_is_lineartile(image->format))
      format = khrn_image_to_tf_format(image->format);
   else
      format = image->format;

   return
      format == ABGR_8888_TF ||
      format == XBGR_8888_TF ||
      format == ARGB_8888_TF ||
      format == XRGB_8888_TF ||
      format == RGBA_8888_TF ||
      format == RGBX_8888_TF ||
      format == RGBA_4444_TF ||
      format == RGBA_5551_TF ||
      format == RGB_565_TF ||
      format == L_8_TF ||
      format == A_8_TF ||
      format == LA_88_TF ||
      format == ETC1_TF ||
      format == PALETTE_4_TF ||

      /* BCG: We do not allow 888 RSO textures - only YUV or 8888
      format == BGR_888_RSO ||
      */
      format == ABGR_8888_RSO ||
      format == ARGB_8888_RSO ||
      format == XBGR_8888_RSO ||
      format == XRGB_8888_RSO ||
      format == RGB_565_RSO ||
      format == YUV_422_RSO ||
      format == YV12_RSO
      ;
}

static bool write_would_block(GLXX_TEXTURE_BLOB_T *item)
{
   /* Write will block if interlock would block, OR if the current interlock is still in flight in hardware */
   return khrn_interlock_write_would_block(&item->interlock) ||
          item->interlock.pos > khrn_get_last_done_seq();
}

static void texture_write_image_immediate(GLXX_TEXTURE_T *texture, uint32_t buffer, uint32_t level)
{
   CHECK_BUFFER(buffer, texture);
   vcos_assert(level <= LOG2_MAX_TEXTURE_SIZE);

   if (texture->mh_mipmaps[buffer][level] != MEM_INVALID_HANDLE) {
      khrn_interlock_write_immediate(&((KHRN_IMAGE_T *)mem_lock(texture->mh_mipmaps[buffer][level], NULL))->interlock);
      mem_unlock(texture->mh_mipmaps[buffer][level]);
   }

   /*
      Even if not pristine, not all mipmap images have necessarily been
      created. So still need to check texture's own read count.
   */

   if(write_would_block(&texture->blob_pool[texture->current_item]))
   {
      /*pick a non busy entry from the pool;*/
      uint32_t i;
      for(i = 0; i< GLXX_TEXTURE_POOL_SIZE; i++)
      {
         if(i!=(uint32_t)texture->current_item && !write_would_block(&texture->blob_pool[i]))
               break;
      }
      if(i<GLXX_TEXTURE_POOL_SIZE)
      {  //found one
         //copy existing data into this new item assuming alloc succeeds
         MEM_HANDLE_T existing_blob = MEM_HANDLE_INVALID;
         //remember existing handle - and retain in case we need to keep it
         assign_retain(&existing_blob,glxx_texture_get_storage_handle(texture));
         //borrow existing function - new blob ends up in current_item
         if(copy_mipmaps_to_new_blob(texture)!=OUT_OF_MEMORY)/* retains new blob for us, unretains old */
         {
            /* assign but not unretain, as should already be unretained */
            MEM_ASSIGN(texture->blob_pool[i].mh_storage,texture->blob_pool[texture->current_item].mh_storage);
            MEM_ASSIGN(texture->blob_pool[texture->current_item].mh_storage,existing_blob); /* assign but not retain */
            assign_retain(&existing_blob,MEM_INVALID_HANDLE);/* unretain */
            texture->current_item = i;
         }
         else
            MEM_ASSIGN(existing_blob,MEM_INVALID_HANDLE); /* lose 1 ref but keep retained */
      } //else stick with the existing and wait
   }

   khrn_interlock_write_immediate(&texture->blob_pool[texture->current_item].interlock);
}

MEM_HANDLE_T glxx_texture_share_mipmap(GLXX_TEXTURE_T *texture, uint32_t buffer, uint32_t level)
{
   CHECK_BUFFER(buffer, texture);
   vcos_assert(level <= LOG2_MAX_TEXTURE_SIZE);

   texture->framebuffer_sharing = true;

   if (texture->mh_mipmaps[buffer][level] == MEM_INVALID_HANDLE && texture->blob_valid[buffer] & 1<<level) {
      /*
         This mipmap exists but has not been shared before. Need to create an image object
         pointing to our blob.
      */
      uint32_t width = _max(texture->width >> level, 1);
      uint32_t height = _max(texture->height >> level, 1);
      MEM_HANDLE_T image = khrn_image_create_from_storage(
         glxx_texture_get_mipmap_format(texture, level, false),
         width,
         height,
         texture_get_mipmap_stride(texture, level, false),
         MEM_INVALID_HANDLE,
         glxx_texture_get_storage_handle(texture),
         glxx_texture_get_mipmap_offset(texture, buffer, level, false),
         IMAGE_CREATE_FLAG_TEXTURE,
         texture->blob_pool[texture->current_item].secure); /* todo: check flags */

      /* TODO: we have no way of reporting the out-of-memory case here */
      if (image != MEM_INVALID_HANDLE) {
         MEM_ASSIGN(texture->mh_mipmaps[buffer][level], image);
         mem_release(image);
         texture->explicit_mipmaps++;
      }
   }

   return texture->mh_mipmaps[buffer][level];
}

static MEM_HANDLE_T * texture_get_storage_handle_ref(GLXX_TEXTURE_T *texture)
{
   return &texture->blob_pool[texture->current_item].mh_storage;
}

MEM_HANDLE_T glxx_texture_get_storage_handle(GLXX_TEXTURE_T *texture)
{
   return texture->blob_pool[texture->current_item].mh_storage;
}

uint32_t glxx_texture_get_interlock_offset(GLXX_TEXTURE_T *texture)
{
   return (uint32_t)((char *)&texture->blob_pool[texture->current_item].interlock - (char *)texture);
}

void glxx_texture_has_color_alpha(GLXX_TEXTURE_T *texture, bool *has_color, bool *has_alpha, bool *complete)
{
   if (glxx_texture_check_complete(texture))
   {
      KHRN_IMAGE_FORMAT_T format = glxx_texture_get_tformat(texture);
      switch (format) {
      case ABGR_8888_TF:
      case ABGR_8888_LT:
      case ARGB_8888_TF:
      case ARGB_8888_LT:
      case RGBA_8888_TF:
      case RGBA_8888_LT:
      case BGRA_8888_TF:
      case BGRA_8888_LT:
      case RGBA_4444_TF:
      case RGBA_4444_LT:
      case RGBA_5551_TF:
      case RGBA_5551_LT:
      case AL_88_TF:
      case AL_88_LT:
      case LA_88_TF:
      case LA_88_LT:
      case PALETTE_4_TF:
      case PALETTE_4_LT:
      case ABGR_8888_RSO:
      case RGBA_8888_RSO:
      case ARGB_8888_RSO:
      case BGRA_8888_RSO:
         *has_color = true;
         *has_alpha = true;
         break;
      case XBGR_8888_TF:
      case XBGR_8888_LT:
      case XRGB_8888_TF:
      case XRGB_8888_LT:
      case RGBX_8888_TF:
      case RGBX_8888_LT:
      case RGB_565_TF:
      case RGB_565_LT:
      case L_8_TF:
      case L_8_LT:
      case ETC1_TF:
      case ETC1_LT:
      case BGR_888_RSO:
      case RGB_888_RSO:
      case RGB_565_RSO:
      case BGR_565_RSO:
      case XBGR_8888_RSO:
      case RGBX_8888_RSO:
      case YUV_422_RSO:
      case YUV_422_REV_RSO:
      case YV12_RSO:
         *has_color = true;
         *has_alpha = false;
         break;
      case A_8_TF:
      case A_8_LT:
         *has_color = false;
         *has_alpha = true;
         break;
      default:
         UNREACHABLE();
         break;
      }
      *complete = true;
   }
   else
   {
      *complete = false;
   }
}

/* GL_OES_draw_texture */
void glxx_texture_set_crop_rect(GLXX_TEXTURE_T *texture, const GLint * params)
{
   /* Ucr, Vcr, Wcr and Hcr */
   texture->crop_rect.Ucr=params[0];
   texture->crop_rect.Vcr=params[1];
   texture->crop_rect.Wcr=params[2];
   texture->crop_rect.Hcr=params[3];
}
