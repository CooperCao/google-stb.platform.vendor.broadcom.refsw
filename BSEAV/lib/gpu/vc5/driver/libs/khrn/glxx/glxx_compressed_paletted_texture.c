/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "libs/core/lfmt_translate_gl/lfmt_translate_gl.h"

#include "glxx_texture.h"
#include "glxx_server_internal.h"
#include "glxx_server_texture.h"

/* compressed paletted texture level description */
struct cpt_level_info
{
   GLsizei width, height;
   uint32_t indices_count;
   size_t offset; /* offset to mipmap level's indices from input data */
};

struct cpt_conv
{
   uint32_t palette_count;
   size_t pixel_size; /* palette pixel size in bytes */
   GLenum packed_type;
   GLenum base_internal_format;
   void (*convert)(const struct cpt_conv *conv, const uint8_t *palette,
      const uint8_t *indices, uint32_t num_indices, uint8_t *pixels);
};

static void convert_paletted_256(const struct cpt_conv *conv,
   const uint8_t *palette, const uint8_t *indices, uint32_t num_indices, uint8_t *dst)
{
   uint32_t i;

   for (i = 0; i < num_indices; i++)
   {
      memcpy(dst, palette + (*indices * conv->pixel_size), conv->pixel_size);
      dst += conv->pixel_size;
      indices++;
   }
}

static void convert_paletted_16(const struct cpt_conv *conv,
   const uint8_t *palette, const uint8_t *indices, uint32_t num_indices, uint8_t *dst)
{
   uint32_t i;
   uint8_t index = *indices;
   bool odd_num_indices = num_indices & 1;

   if (odd_num_indices)
      num_indices -= 1;

   for (i = 0; i < num_indices; i += 2)
   {
      memcpy(dst, palette + ((index >> 4) * conv->pixel_size), conv->pixel_size);
      dst += conv->pixel_size;
      memcpy(dst, palette + ((index & 0xF) * conv->pixel_size), conv->pixel_size);
      dst += conv->pixel_size;
      indices++;
      index = *indices;
   }

   if (odd_num_indices)
      memcpy(dst, palette + ((index >> 4) * conv->pixel_size), conv->pixel_size);

}

static const struct cpt_conv cpt_conv_table[] =
{
/*0: GL_PALETTE4_RGB8_OES              0x8B90*/
   { 16, 3, GL_UNSIGNED_BYTE, GL_RGB, convert_paletted_16 },
/*1: GL_PALETTE4_RGBA8_OES             0x8B91*/
   { 16, 4, GL_UNSIGNED_BYTE, GL_RGBA, convert_paletted_16 },
/*2: GL_PALETTE4_R5_G6_B5_OES          0x8B92*/
   { 16, 2, GL_UNSIGNED_SHORT_5_6_5, GL_RGB, convert_paletted_16 },
/*3: GL_PALETTE4_RGBA4_OES             0x8B93*/
   { 16, 2, GL_UNSIGNED_SHORT_4_4_4_4, GL_RGBA, convert_paletted_16 },
/*4: GL_PALETTE4_RGB5_A1_OES           0x8B94*/
   { 16, 2, GL_UNSIGNED_SHORT_5_5_5_1, GL_RGBA, convert_paletted_16 },
/*5: GL_PALETTE8_RGB8_OES              0x8B95*/
   { 256, 3, GL_UNSIGNED_BYTE, GL_RGB, convert_paletted_256 },
/*6: GL_PALETTE8_RGBA8_OES             0x8B96*/
   { 256, 4, GL_UNSIGNED_BYTE, GL_RGBA, convert_paletted_256 },
/*7: GL_PALETTE8_R5_G6_B5_OES          0x8B97*/
   { 256, 2, GL_UNSIGNED_SHORT_5_6_5, GL_RGB, convert_paletted_256 },
/*8: GL_PALETTE8_RGBA4_OES             0x8B98*/
   { 256, 2, GL_UNSIGNED_SHORT_4_4_4_4, GL_RGBA, convert_paletted_256 },
/*9: GL_PALETTE8_RGB5_A1_OES           0x8B99*/
   { 256, 2, GL_UNSIGNED_SHORT_5_5_5_1, GL_RGBA, convert_paletted_256 },
};

static const struct cpt_conv *get_cpt_conv(GLenum format)
{
   assert(format >= GL_PALETTE4_RGB8_OES && format <= GL_PALETTE8_RGB5_A1_OES);
   return &cpt_conv_table[format - GL_PALETTE4_RGB8_OES];
}

static size_t base_image_uncompressed_size(const struct cpt_conv *conv,
   unsigned width, unsigned height)
{
   GFX_BUFFER_DESC_T desc;
   size_t size, align;
   GFX_LFMT_T plane_lfmt = gfx_lfmt_from_externalformat(conv->base_internal_format,
         conv->packed_type, conv->base_internal_format);

   gfx_lfmt_set_dims(&plane_lfmt, GFX_LFMT_DIMS_2D);

   gfx_buffer_desc_gen(&desc, &size, &align,
      0,
      width, height, 1,
      1, 1, &plane_lfmt);

   assert (conv->pixel_size * width == desc.planes[0].pitch);
   return size;
}

static size_t size_of_palette(const struct cpt_conv *conv)
{
   return conv->pixel_size * conv->palette_count;
}

static size_t size_of_indices(const struct cpt_conv *conv, unsigned count)
{
   if (conv->palette_count == 16)
      count = (count / 2) + (count & 1);
   return count;
}

static void convert_paletted(const struct cpt_conv *conv,
   const GLvoid *original_pixels, GLvoid *pixels,
   const struct cpt_level_info *level_info)
{
   uint32_t num_indices = level_info->indices_count;
   const uint8_t *palette = original_pixels;
   const uint8_t *indices = palette + level_info->offset;

   conv->convert(conv, palette, indices, num_indices, pixels);
}

static const void *map_src_pixels(GLXX_BUFFER_T *pixel_buffer, const void *ptr,
                                  size_t imageSize, GLenum *error)
{
   size_t buf_offset, buf_size;
   const void *src_ptr;

   if (!pixel_buffer)
      return ptr;

   /* ptr is an offset to pixel buffer object */
   buf_offset = (uintptr_t)ptr;
   buf_size = glxx_buffer_get_size(pixel_buffer);
   if (buf_offset + imageSize < buf_offset ||
       buf_offset + imageSize > buf_size)
   {
       *error = GL_INVALID_OPERATION;
       return NULL;
   }

   src_ptr = glxx_buffer_map_range(pixel_buffer, buf_offset, imageSize, GL_MAP_READ_BIT);
   if (!src_ptr)
   {
      *error = GL_OUT_OF_MEMORY;
      return NULL;
   }

   return src_ptr;
}

static void unmap_src_pixels(GLXX_BUFFER_T *pixel_buffer, const void *ptr,
                             GLsizei imageSize, const void *mapped_ptr)
{
   if (mapped_ptr != ptr)
      glxx_buffer_unmap_range(pixel_buffer, (uintptr_t)ptr, imageSize, GL_MAP_READ_BIT);
}

static bool gen_cpt_level_info(unsigned width, unsigned height,
      unsigned level, size_t imageSize,
      const struct cpt_conv *conv, struct cpt_level_info **level_info, GLenum *error)
{
   unsigned i;
   size_t offset = 0;

   assert(level < KHRN_MAX_MIP_LEVELS);

   *level_info = calloc(level + 1, sizeof(struct cpt_level_info));
   if (!*level_info)
   {
      *error = GL_OUT_OF_MEMORY;
      return false;
   }

   if (width > 0 && height > 0)
   {
      offset = size_of_palette(conv);
      for (i = 0; i <= level; i++)
      {
         struct cpt_level_info *d = *level_info + i;

         d->width = gfx_umax(width >> i, 1);
         d->height = gfx_umax(height >> i, 1);
         d->indices_count = d->width * d->height;
         d->offset = offset;
         offset += size_of_indices(conv, d->indices_count);
      }
   }

   if (imageSize != offset)
   {
      *error = GL_INVALID_VALUE;
      return false;
   }
   return true;
}

void glxx_compressed_paletted_teximageX(GLenum target, GLint level,
      GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border,
      GLsizei imageSize, const GLvoid *orig_pixels, unsigned dim)
{
   GLXX_SERVER_STATE_T *state   = glxx_lock_server_state(OPENGL_ES_ANY);
   GLXX_TEXTURE_T *texture = NULL;
   GLXX_BUFFER_T *pixel_buffer = NULL;
   GLenum error = GL_NO_ERROR;
   void *pixels = NULL;
   const void *mapped_ptr = NULL;
   const struct cpt_conv *conv;
   bool ok;
   unsigned face, i;
   struct glxx_teximage_sanity_checks checks;
   struct cpt_level_info *level_info = NULL;

   if (!state)
      return;

   if (imageSize < 0)
   {
      error = GL_INVALID_OPERATION;
      goto end;
   }

   /* Negative level values are used to define the number of miplevels described
    * in the "data" component.  A level of zero indicates a single mip-level. */
   if (level > 0)
   {
      error = GL_INVALID_VALUE;
      goto end;
   }

   checks.dimensions = dim;
   checks.compressed = true;
   checks.respecify = true;
   level = -level;

   if (!glxx_teximage_internal_checks(state, &checks,
           target, 0, 0, internalformat, level, width, height, depth, 0, 0, 0,
           border, &texture, &error))
      goto end;

   conv = get_cpt_conv(internalformat);
   if (!gen_cpt_level_info(width, height, level, (size_t)imageSize, conv,
            &level_info, &error))
      goto end;

   if (imageSize > 0)
   {
      pixels = malloc(base_image_uncompressed_size(conv, width, height));
      if (!pixels)
      {
         error = GL_OUT_OF_MEMORY;
         goto end;
      }

      pixel_buffer = state->bound_buffer[GLXX_BUFTGT_PIXEL_UNPACK].obj;
      mapped_ptr = map_src_pixels(pixel_buffer, orig_pixels, (size_t)imageSize,
            &error);
      if (!mapped_ptr)
         goto end;
   }

   face = glxx_texture_get_face(target);
   ok = true;
   for (i = 0; i <= (unsigned)level && ok; i++)
   {
      const struct cpt_level_info *d = level_info + i;
      convert_paletted(conv, mapped_ptr, pixels, d);
      ok = glxx_texture_image(texture, face, i, conv->base_internal_format, d->width, d->height,
         depth, conv->base_internal_format, conv->packed_type, &(state->pixel_store_state),
         NULL, pixels, &state->fences, &error);
   }

   if (!ok)
   {
      unsigned j;
      /* release all the levels created up till now and fail */
      for( j = 0; j < i; j++)
         KHRN_MEM_ASSIGN(texture->img[face][j], NULL);
   }

end:
   if (error != GL_NO_ERROR)
      glxx_server_state_set_error(state, error);
   if (pixel_buffer && mapped_ptr)
      unmap_src_pixels(pixel_buffer, orig_pixels, (size_t)imageSize, mapped_ptr);
   free(pixels);
   free(level_info);
   glxx_unlock_server_state();
}
