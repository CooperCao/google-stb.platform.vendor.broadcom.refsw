/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "load_texture.h"

#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <assert.h>

/* required for the BSTD_CPU_ENDIAN == BSTD_ENDIAN_BIG macro to work */
#include "nexus_platform.h"

#include "png.h"


#if BSTD_CPU_ENDIAN == BSTD_ENDIAN_BIG
#define BSWAP32(a) (a)
#else

#if defined(_MSC_VER)
#include <stdlib.h>
#define BSWAP32 _byteswap_ulong
#elif defined(__GNUC__)
#if __GNUC__ > 4 || (__GNUC__ == 4 &&  __GNUC_MINOR__ > 2)
#define BSWAP32 __builtin_bswap32
#else
#define BSWAP32 static_bswap32

static unsigned int static_bswap32(unsigned int b)
{
   return (b & 0x000000ff) << 24 |
      (b & 0x0000ff00) << 8  |
      (b & 0x00ff0000) >> 8  |
      (b & 0xff000000) >> 24;
}
#endif
#else
#endif

#endif /* BSTD_CPU_ENDIAN==BSTD_ENDIAN_BIG */

#define ETC1_RGB_NO_MIPMAPS 0

int LoadPKM(const char *fname, PKMImage *data)
{
   /* Load Compressed file and use as texture */
   FILE *fp;
   unsigned char magic[4];
   unsigned char version[2];
   unsigned int width;
   unsigned int height;
   int status = 0;

   fp = fopen(fname, "rb");
   if (fp == NULL)
      goto error0;

   /* Read magic number */
   fread(magic, sizeof(unsigned char), 4, fp);
   if ((magic[0] != 'P') ||
       (magic[1] != 'K') ||
       (magic[2] != 'M') ||
       (magic[3] != ' '))
   {
#if 1
      printf("\n\n The file %s is not a .pkm file.\n", fname);
#endif
      goto error1;
   }

   /* Read version */
   fread(version, sizeof(unsigned char), 2, fp);
   if ((version[0] != '1') || (version[1] != '0'))
   {
#if 1
      printf("\n\n The file %s is not of version 1.0 but of version %c.%c.\n", fname, version[0], version[1]);
#endif
      goto error1;
   }

   /* Read texture type */
   fread(&data->mFormat, sizeof(unsigned char), 2, fp);
   /* byteswap it */
   data->mFormat = BSWAP32(data->mFormat) >> 16;

   if (!(data->mFormat == ETC1_RGB_NO_MIPMAPS))
   {
#if 1
      printf("\n\n The file %s does not contain a texture of known format.\n", fname);
      printf("Known formats: ETC1_RGB_NO_MIPMAPS.\n");
#endif
      goto error1;
   }

   fread(&width, sizeof(unsigned char), 2, fp);
   width = BSWAP32(width) >> 16;
   fread(&height, sizeof(unsigned char), 2, fp);
   height = BSWAP32(height) >> 16;

   fread(&data->mWidth, sizeof(unsigned char), 2, fp);
   data->mWidth = BSWAP32(data->mWidth) >> 16;
   fread(&data->mHeight, sizeof(unsigned char), 2, fp);
   data->mHeight = BSWAP32(data->mHeight) >> 16;

   data->mSize = (width * height * 4) / 8;

   /* allocate enough for 4bpp */
   data->mBytes = (unsigned char *)malloc(data->mSize);
   if (data->mBytes)
   {
      fread(data->mBytes, sizeof(char), data->mSize, fp);
      status = 1;
   }

   /* success! fallthrough here, close file and quit return 1 */

error1:

   fclose(fp);

error0:

   return status;
}

void FreePKM(PKMImage *data)
{
   if (data->mBytes != NULL)
      free(data->mBytes);
   memset(data, 0, sizeof(PKMImage));
}

static unsigned short dither_rgb565(unsigned char r8, unsigned char g8, unsigned char b8,
                                    unsigned int x, unsigned int y)
{
   static const int dither_matrix[16] =
   {
      0, 4, 1, 5,
      6, 2, 7, 3,
      1, 5, 0, 4,
      7, 3, 6, 2
   };

   static const int dither_matrix_5[16] =
   {
      0,  8,  2, 10,
      4, 12,  6, 14,
      1,  9,  3, 11,
      5, 13,  7, 15
   };

   unsigned int index;
   int dither, dither_5;
   int r, g, b;

   index = (x & 3) + ((y & 3)<<2);

   dither = dither_matrix[index];
   dither_5 = dither_matrix_5[index];

   r = r8;
   g = g8;
   b = b8;

   r = (r<<1) | 1;   /* 255.5 */
   g = (g<<1) | 1;
   b = (b<<1) | 1;

   r = r - (r >> 5) + (dither_5 | 1);
   g = g - (g >> 6) + (dither   | 1);
   b = b - (b >> 5) + (dither_5 | 1);

   r = r >> 4;
   g = g >> 3;
   b = b >> 4;

   assert(r >= 0 && g >= 0 && b >= 0 && r < 32 && g < 64 && b < 32);
   return r << 11 | g << 5 | b;
}

static unsigned short dither_rgb5551(unsigned char r8, unsigned char g8, unsigned char b8, unsigned char a8,
                                     unsigned int x, unsigned int y)
{
   static const int dither_matrix[16] =
   {
      0,  8,  2, 10,
      4, 12,  6, 14,
      1,  9,  3, 11,
      5, 13,  7, 15
   };

   unsigned int index;
   int dither;
   int r, g, b, a;

   index = (x & 3) + ((y & 3)<<2);

   dither = dither_matrix[index];

   r = r8;
   g = g8;
   b = b8;
   a = a8;

   a = (a != 0);
   r = (r<<1) | 1;
   g = (g<<1) | 1;
   b = (b<<1) | 1;

   r = r - (r >> 5) + (dither | 1);
   g = g - (g >> 5) + (dither | 1);
   b = b - (b >> 5) + (dither | 1);

   r = r >> 4;
   g = g >> 4;
   b = b >> 4;

   assert(r >= 0 && g >= 0 && b >= 0 && r < 32 && g < 32 && b < 32);
   return r << 11 | g << 6 | b << 1 | a;
}

static unsigned short dither_rgb4444(unsigned char r8, unsigned char g8, unsigned char b8, unsigned char a8,
                                     unsigned int x, unsigned int y)
{
   static const int dither_matrix[64] =
   {
      0, 16,  4, 20,  1, 17,  5, 21,
      8, 24, 12, 28,  9, 25, 13, 29,
      2, 18,  6, 22,  3, 19,  7, 23,
      10, 26, 14, 30, 11, 27, 15, 31,
      1, 17,  3, 19,  0, 16,  2, 18,
      9, 25, 11, 27,  8, 24, 10, 26,
      5, 21,  7, 23,  4, 20,  6, 22,
      13, 29, 15, 31, 12, 28, 14, 30,
   };

   unsigned int index;
   int dither;
   int r, g, b, a;

   index = (x & 7) + ((y & 7)<<3);

   dither = dither_matrix[index];

   r = r8;
   g = g8;
   b = b8;
   a = a8;

   a = (a<<1) | 1;
   r = (r<<1) | 1;
   g = (g<<1) | 1;
   b = (b<<1) | 1;

   a = a - (a >> 4) + (dither | 1);
   r = r - (r >> 4) + (dither | 1);
   g = g - (g >> 4) + (dither | 1);
   b = b - (b >> 4) + (dither | 1);

   a = a >> 5;
   r = r >> 5;
   g = g >> 5;
   b = b >> 5;

   assert(r >= 0 && g >= 0 && b >= 0 && a >= 0 && r < 16 && g < 16 && b < 16 && a < 16);
   return r << 12 | g << 8 | b << 4 | a;
}

int LoadPNG(const char *fname, int dest_format, PNGImage *pngdata)
{
   int status = 0;

   /* open file and test for it being a png */
   FILE *fp = fopen(fname, "rb");
   if (!fp)
      goto error0;

   char header[8];               /* 8 is the maximum size that can be checked */
   fread(header, 1, 8, fp);
   if (png_sig_cmp((png_bytep)header, 0, 8))
      goto error0;

   /* initialize stuff */
   png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
   if (!png_ptr)
      goto error1;

   png_infop info_ptr = png_create_info_struct(png_ptr);
   if (!info_ptr)
      goto error1;

   if (setjmp(png_jmpbuf(png_ptr)))
      goto error1;

   png_init_io(png_ptr, fp);
   png_set_sig_bytes(png_ptr, 8);

   png_read_info(png_ptr, info_ptr);

   png_set_interlace_handling(png_ptr);
   png_read_update_info(png_ptr, info_ptr);

   png_byte pixel_depth = png_get_bit_depth(png_ptr, info_ptr) * png_get_channels(png_ptr, info_ptr);
   png_byte color_type = png_get_color_type(png_ptr, info_ptr);

   if ((color_type == PNG_COLOR_TYPE_GRAY_ALPHA) && (pixel_depth == 0x10))
   {
      png_set_expand(png_ptr);
      png_set_gray_to_rgb(png_ptr);
      png_read_update_info(png_ptr, info_ptr);
   }
   else if ((color_type == PNG_COLOR_TYPE_RGB) && (pixel_depth == 0x18))
   {
      png_set_expand(png_ptr);
      png_set_add_alpha(png_ptr, 0xff, PNG_FILLER_AFTER);
      png_read_update_info(png_ptr, info_ptr);
   }

   int w = png_get_image_width(png_ptr, info_ptr);
   int h = png_get_image_height(png_ptr, info_ptr);

   png_bytep *row_pointers = (png_bytep*)malloc(sizeof(png_bytep) * h);
   if (!row_pointers)
      goto error1;

   /* allocate enough memory for the entire image */
   row_pointers[h - 1] = (png_byte*)malloc(h * png_get_rowbytes(png_ptr,info_ptr));
   if (!row_pointers[h - 1])
      goto error2;

   /* load the image upside down for OpenGL */
   for (int y=h-1; y>0; y--)
      row_pointers[y-1] = (png_byte*)(row_pointers[y] + png_get_rowbytes(png_ptr,info_ptr));

   /* read file */
   if (setjmp(png_jmpbuf(png_ptr)))
      goto error3;

   png_read_image(png_ptr, row_pointers);

   png_read_end(png_ptr, info_ptr);

   fclose(fp);

   pngdata->mWidth = w;
   pngdata->mHeight = h;
   pngdata->mFormat = (DEST_FORMAT)dest_format;

   /* convert our GL_RGBA to something else */
   if (dest_format == DF_565)
   {
      pngdata->mStride = w * sizeof(unsigned short);

      unsigned char *in = (unsigned char *)row_pointers[h - 1];
      pngdata->mBytes = (unsigned char *)malloc(pngdata->mStride * h);
      unsigned short *out = (unsigned short*)pngdata->mBytes;

      if (!pngdata->mBytes)
         goto error3;

      /* dither and truncate */
      for (int y = 0; y < h; y++)
      {
         for (int x = 0; x < w; x++)
         {
            unsigned char *ptr = &in[4 * (y * w + x)];
            out[y * w + x] = dither_rgb565(ptr[0], ptr[1], ptr[2], x, y);
         }
      }
   }
   else if (dest_format == DF_4444)
   {
      pngdata->mStride = w * sizeof(unsigned short);

      unsigned char *in = (unsigned char *)row_pointers[h - 1];
      pngdata->mBytes = (unsigned char *)malloc(pngdata->mStride * h);
      unsigned short *out = (unsigned short*)pngdata->mBytes;
      if (!out)
         goto error3;

      /* dither and truncate */
      for (int y = 0; y < h; y++)
      {
         for (int x = 0; x < w; x++)
         {
            unsigned char *ptr = &in[4 * (y * w + x)];
            out[y * w + x] = dither_rgb4444(ptr[0], ptr[1], ptr[2], ptr[3], x, y);
         }
      }
   }
   else if (dest_format == DF_5551)
   {
      pngdata->mStride = w * sizeof(unsigned short);

      unsigned char *in = (unsigned char *)row_pointers[h - 1];
      pngdata->mBytes = (unsigned char *)malloc(pngdata->mStride * h);
      unsigned short *out = (unsigned short*)pngdata->mBytes;
      if (!out)
         goto error3;

      /* dither and truncate */
      for (int y = 0; y < h; y++)
      {
         for (int x = 0; x < w; x++)
         {
            unsigned char *ptr = &in[4 * (y * w + x)];
            out[y * w + x] = dither_rgb5551(ptr[0], ptr[1], ptr[2], ptr[3], x, y);
         }
      }
   }
   else if (dest_format == DF_8888)
   {
      pngdata->mStride = w * 4 * sizeof(unsigned char);
      pngdata->mBytes = (unsigned char *)malloc(pngdata->mStride * h);
      if (!pngdata->mBytes)
         goto error3;
      memcpy(pngdata->mBytes, row_pointers[h - 1], pngdata->mStride * h);
   }
   else if (dest_format == DF_888)
   {
      /* GL uses default packing of 4 bytes */
      pngdata->mStride = ((w * 3) + 3) & ~0x3;

      unsigned char *in = (unsigned char *)row_pointers[h - 1];
      pngdata->mBytes = (unsigned char *)malloc(pngdata->mStride * h);
      unsigned char *out = (unsigned char*)pngdata->mBytes;
      if (!out)
         goto error3;

      /* dither and truncate */
      for (int y = 0; y < h; y++)
      {
         for (int x = 0; x < w; x++)
         {
            out[x * 3] = in[x * 4];
            out[x * 3 + 1] = in[x * 4 + 1];
            out[x * 3 + 2] = in[x * 4 + 2];
         }
         in = in + (w * 4);
         out = out + pngdata->mStride;
      }
   }
   else if (dest_format == DF_ALPHA)
   {
      /* GL uses default packing of 4 bytes */
      pngdata->mStride = (w + 3) & ~0x3;

      unsigned char *in = (unsigned char *)row_pointers[h - 1];
      pngdata->mBytes = (unsigned char *)malloc(pngdata->mStride * h);
      unsigned char *out = (unsigned char*)pngdata->mBytes;
      if (!out)
         goto error3;

      /* just take the alpha */
      for (int y = 0; y < h; y++)
      {
         for (int x = 0; x < w; x++)
         {
            out[x] = in[x * 4 + 3];
         }
         in = in + (w * 4);
         out = out + pngdata->mStride;
      }
   }
   else
      goto error3;

   status = 1;

error3:
   free(row_pointers[h - 1]);
error2:
   free(row_pointers);
error1:
   png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
error0:
   return status;
}

void FreePNG(PNGImage *data)
{
   if (data->mBytes != NULL)
      free(data->mBytes);
   memset(data, 0, sizeof(PNGImage));
}
