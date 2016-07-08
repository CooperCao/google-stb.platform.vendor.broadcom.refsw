/******************************************************************************
 *   Broadcom Proprietary and Confidential. (c)2011-2012 Broadcom.  All rights reserved.
 *
 * This program is the proprietary software of Broadcom and/or its
 * licensors, and may only be used, duplicated, modified or distributed
 * pursuant to the terms and conditions of a separate, written license
 * agreement executed between you and Broadcom (an "Authorized License").
 * Except as set forth in an Authorized License, Broadcom grants no license
 * (express or implied), right to use, or waiver of any kind with respect to
 * the Software, and Broadcom expressly reserves all rights in and to the
 * Software and all intellectual property rights therein.  IF YOU HAVE NO
 * AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY,
 * AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE
 * SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use all
 * reasonable efforts to protect the confidentiality thereof, and to use this
 * information only in connection with your use of Broadcom integrated circuit
 * products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 * OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 * RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 * IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 * A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 * ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE
 * ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
 * ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 * INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 * RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 * HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 * EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 * WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 * FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *****************************************************************************/

#ifndef BSG_STAND_ALONE

#include "bsg_image_png.h"
#include "bsg_exception.h"
#include "bsg_dither.h"
#include "bsg_trackers.h"

#include "png.h"

#include <string>
#include <string.h>
#include <stdlib.h>

using namespace std;

namespace bsg
{

// This code should probably exist as a helper library
typedef union {
   float    f;
   uint32_t i;
} UintFloat32;

static inline uint32_t FloatToUint32(float f)
{
   UintFloat32 t;
   t.f = f;
   return t.i;
}

#ifdef BSG_USE_ES3
// Note this is not a general float->half float conversion
// But here we are only interested in values from 0.0f to 1.0f
static uint16_t FloatToFP16(float f)
{
   if (f == 0.0)
      return 0;

   uint32_t bits = FloatToUint32(f);

   return ((bits >> 16) & 0x8000) | ((((bits & 0x7f800000) - 0x38000000) >> 13) & 0x7c00) | ((bits >> 13) & 0x03ff);
}
#endif

namespace prv
{
class DestroyReadStruct
{
public:
   DestroyReadStruct(png_structp *pngPtr, png_infop *infoPtr) :
      m_pngPtr(pngPtr),
      m_infoPtr(infoPtr)
   {}

   ~DestroyReadStruct()
   {
      if (*m_pngPtr != 0)
         png_destroy_read_struct(m_pngPtr, m_infoPtr, NULL);
   }

private:
   png_structp *m_pngPtr;
   png_infop   *m_infoPtr;
};

}

ImagePNG::ImagePNG(const string &fileName, Image::eFormat format)
{
   png_structp png_ptr  = NULL;
   png_infop   info_ptr = NULL;
   png_bytep   *row_pointers;
   char        header[8];                          // 8 is the maximum size that can be checked

   prv::DestroyReadStruct  destroyRS(&png_ptr, &info_ptr);    // Destroy the read structures when exit

   // open file and test for it being a png
   FILE *fp = fopen(fileName.c_str(), "rb");

   if (!fp)
      BSG_THROW("Cannot find file " << fileName);

   FClose   fcloseFP(fp);                       // Closes file on exit

   if (fread(header, 1, 8, fp) != 8)
      BSG_THROW("File " << fileName << "is not a valid PNG file");

   if (png_sig_cmp((png_bytep)header, 0, 8))
      BSG_THROW("File " << fileName << "is not a valid PNG file");

   // initialize stuff
   png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

   if (!png_ptr)
      BSG_THROW("Cannot create png read structure");

   info_ptr = png_create_info_struct(png_ptr);
   if (!info_ptr)
      BSG_THROW("Cannot create png info structure");

#if PNG_LIBPNG_VER >= 10006
   if (setjmp(png_jmpbuf(png_ptr)))
      BSG_THROW("setjmp() failed");
#else
   if (setjmp(png_ptr->jmpbuf))
      BSG_THROW("setjmp() failed");
#endif

   png_init_io(png_ptr, fp);
   png_set_sig_bytes(png_ptr, 8);

   png_read_info(png_ptr, info_ptr);
   png_set_interlace_handling(png_ptr);

   png_byte pixel_depth = png_get_bit_depth(png_ptr, info_ptr) * png_get_channels(png_ptr, info_ptr);
   png_byte color_type  = png_get_color_type(png_ptr, info_ptr);

   if ((color_type == PNG_COLOR_TYPE_GRAY_ALPHA) && (pixel_depth == 0x10))
   {
      png_set_expand(png_ptr);
      png_set_gray_to_rgb(png_ptr);
   }
   else if ((color_type == PNG_COLOR_TYPE_RGB) && (pixel_depth == 0x18))
   {
      png_set_expand(png_ptr);
      png_set_add_alpha(png_ptr, 0xff, PNG_FILLER_AFTER);
   }

   png_read_update_info(png_ptr, info_ptr);

   m_width          = png_get_image_width(png_ptr, info_ptr);
   m_height         = png_get_image_height(png_ptr, info_ptr);

   /* grab rowbytes here as it could have been altered above */
   png_size_t rowbytes = png_get_rowbytes(png_ptr, info_ptr);
   /* re-read pixel depth, as this could have also changed */
   pixel_depth  = png_get_bit_depth(png_ptr, info_ptr) * png_get_channels(png_ptr, info_ptr);

   row_pointers = (png_bytep*)malloc(sizeof(png_bytep) * m_height);
   if (row_pointers == 0)
      BSG_THROW("Out of memory");

   // Mark row_pointers for freeing
   Free  freeRowPointers(row_pointers);

   // allocate enough memory for the entire image
   png_byte *image_ptr = (png_byte*)malloc(m_height * rowbytes);
   if (image_ptr == 0)
      BSG_THROW("Out of memory");

   row_pointers[m_height - 1] = image_ptr;

   // Mark for freeing
   Free  freeImage(image_ptr);

   // load the image upside down for OpenGL
   for (uint32_t y = m_height - 1; y > 0; y--)
      row_pointers[y - 1] = (png_byte*)(row_pointers[y] + rowbytes);

   // read file
#if (PNG_LIBPNG_VER >= 10006)
   if (setjmp(png_jmpbuf(png_ptr)))
      BSG_THROW("setjmp() failed");
#else
   if (setjmp(png_ptr->jmpbuf))
      BSG_THROW("setjmp() failed");
#endif

   png_read_image(png_ptr, row_pointers);
   png_read_end(png_ptr, info_ptr);

   m_internalFormat = Image::InternalFormat(format);
   m_format         = Image::Format(format);
   m_type           = Image::Type(format);

   uint32_t stride;

   // THIS CODE CRIES OUT FOR TEMPLATING

   // convert our GL_RGBA to something else
   if (format == Image::eRGB565)
   {
      uint16_t *out;
      uint32_t *in;

      stride = m_width * sizeof(uint16_t);
      m_size = stride * m_height;

      in     = (uint32_t *)image_ptr;
      m_data = (char *)    malloc(m_size);
      out    = (uint16_t *)m_data;

      if (m_data == 0)
         BSG_THROW("Out of memory");

      // dither and truncate
      for (uint32_t y = 0; y < m_height; y++)
      {
         for (uint32_t x = 0; x < m_width; x++)
         {
            uint8_t *ptr = (uint8_t*)&in[y * m_width + x];
            out[y * m_width + x] = DitherRgb565(ptr[0], ptr[1], ptr[2], x, y);
         }
      }
   }
   else if (format == Image::eRGBA4444)
   {
      uint16_t *out;
      uint32_t *in;

      stride = m_width * sizeof(uint16_t);
      m_size = stride * m_height;

      in     = (uint32_t *)image_ptr;
      m_data = (char *)    malloc(m_size);
      out    = (uint16_t*) m_data;

      if (m_data == 0)
         BSG_THROW("Out of memory");

      // dither and truncate
      for (uint32_t y = 0; y < m_height; y++)
      {
         for (uint32_t x = 0; x < m_width; x++)
         {
            uint8_t *ptr = (uint8_t*)&in[y * m_width + x];
            out[y * m_width + x] = DitherRgb4444(ptr[0], ptr[1], ptr[2], ptr[3], x, y);
         }
      }
   }
   else if (format == Image::eRGBA5551)
   {
      uint16_t * out;
      uint32_t * in;

      stride = m_width * sizeof(uint16_t);
      m_size = stride * m_height;

      in     = (uint32_t *)image_ptr;
      m_data = (char *)malloc(m_size);
      out    = (uint16_t*)m_data;

      if (m_data == 0)
         BSG_THROW("Out of memory");

      /* dither and truncate */
      for (uint32_t y = 0; y < m_height; y++)
      {
         for (uint32_t x = 0; x < m_width; x++)
         {
            uint8_t *ptr = (uint8_t*)&in[y * m_width + x];
            out[y * m_width + x] = DitherRgb5551(ptr[0], ptr[1], ptr[2], ptr[3], x, y);
         }
      }
   }
   else if (format == Image::eRGBA8888)
   {
      stride = m_width * 4 * sizeof(unsigned char);
      m_size = stride * m_height;

      m_data = (char *)malloc(m_size);

      if (m_data == 0)
         BSG_THROW("Out of memory");

      memcpy(m_data, image_ptr, stride * m_height);
   }
   else if (format == Image::eRGB888)
   {
      unsigned char  *out;
      unsigned char  *in;

      stride = ((m_width * 3) + 3) & ~0x3;
      m_size = stride * m_height;

      in     = (unsigned char *)image_ptr;
      m_data = (char *)         malloc(m_size);
      out    = (unsigned char*) m_data;

      if (m_data == 0)
         BSG_THROW("Out of memory");

      // dither and truncate
      for (uint32_t y = 0; y < m_height; y++)
      {
         for (uint32_t x = 0; x < m_width; x++)
         {
            out[x * 3]     = in[x * 4];
            out[x * 3 + 1] = in[x * 4 + 1];
            out[x * 3 + 2] = in[x * 4 + 2];
         }
         out += stride;
         in += m_width * 4;
      }
   }
   else if (format == Image::eA8)
   {
      unsigned char  *out;
      unsigned char  *in;

      stride = (m_width + 3) & ~0x3;
      m_size = stride * m_height;

      in     = (unsigned char *)image_ptr;
      m_data = (char *)         malloc(m_size);
      out    = (unsigned char*) m_data;

      if (m_data == 0)
         BSG_THROW("Out of memory");

      // Grab alpha channel
      for (uint32_t y = 0; y < m_height; y++)
      {
         for (uint32_t x = 0; x < m_width; x++)
            out[x]     = in[x * 4 + 3];

         out += stride;
         in += m_width * 4;
      }
   }
   else if (format == Image::eL8)
   {
      unsigned char  *out;
      unsigned char  *in;

      stride = (m_width + 3) & ~0x3;
      m_size = stride * m_height;

      in     = (unsigned char *)image_ptr;
      m_data = (char *)         malloc(m_size);
      out    = (unsigned char*) m_data;

      if (m_data == 0)
         BSG_THROW("Out of memory");

      // Convert to lumance 0.3 R + 0.59 G + 0.11 B
      for (uint32_t y = 0; y < m_height; y++)
      {
         for (uint32_t x = 0; x < m_width; x++)
            out[x] = (unsigned char)(in[x * 4 + 0] * 0.3f + in[x * 4 + 1] * 0.59f + in[x * 4 + 2] * 0.11f);

         out += stride;
         in += m_width * 4;
      }
   }
   else if (format == Image::eLA88)
   {
      unsigned char  *out;
      unsigned char  *in;

      stride = ((m_width * 2) + 3) & ~0x3;
      m_size = stride * m_height;

      in     = (unsigned char *)image_ptr;
      m_data = (char *)         malloc(m_size);
      out    = (unsigned char*) m_data;

      if (m_data == 0)
         BSG_THROW("Out of memory");

      // Convert to lumance 0.3 R + 0.59 G + 0.11 B
      // Copy alpha
      for (uint32_t y = 0; y < m_height; y++)
      {
         for (uint32_t x = 0; x < m_width; x++)
         {
            out[x * 2 + 0] = (unsigned char)(in[x * 4 + 0] * 0.3f + in[x * 4 + 1] * 0.59f + in[x * 4 + 2] * 0.11f);
            out[x * 2 + 1] = in[x * 4 + 3];
         }

         out += stride;
         in += m_width * 4;
      }
   }
#ifdef BSG_USE_ES3
   else if (format == Image::eRGBA16I || format == Image::eRGBA16UI)
   {
      uint16_t       *out;
      unsigned char  *in;

      stride = m_width * 8;
      m_size = stride * m_height;

      in     = (unsigned char *)image_ptr;
      m_data = (char *)         malloc(m_size);
      out    = (uint16_t *)     m_data;

      if (m_data == 0)
         BSG_THROW("Out of memory");

      for (uint32_t y = 0; y < m_height; y++)
      {
         for (uint32_t x = 0; x < m_width; x++)
         {
            uint32_t i = x * 4;

            out[i + 0] = (uint16_t)in[i + 0];
            out[i + 1] = (uint16_t)in[i + 1];
            out[i + 2] = (uint16_t)in[i + 2];
            out[i + 3] = (uint16_t)in[i + 3];
         }

         out += m_width * 4;
         in  += m_width * 4;
      }
   }
   else if (format == Image::eRGBA16F)
   {
      uint16_t       *out;
      unsigned char  *in;

      stride = m_width * 8;
      m_size = stride * m_height;

      in     = (unsigned char *)image_ptr;
      m_data = (char *)         malloc(m_size);
      out    = (uint16_t *)     m_data;

      if (m_data == 0)
         BSG_THROW("Out of memory");

      for (uint32_t y = 0; y < m_height; y++)
      {
         for (uint32_t x = 0; x < m_width; x++)
         {
            uint32_t i = x * 4;

            out[i + 0] = FloatToFP16((float)in[i + 0] / 255.0f);
            out[i + 1] = FloatToFP16((float)in[i + 1] / 255.0f);
            out[i + 2] = FloatToFP16((float)in[i + 2] / 255.0f);
            out[i + 3] = FloatToFP16((float)in[i + 3] / 255.0f);
         }

         out += m_width * 4;
         in  += m_width * 4;
      }
   }
   else if (format == Image::eRGBA32I || format == Image::eRGBA32UI)
   {
      uint32_t       *out;
      unsigned char  *in;

      stride = m_width * 16;
      m_size = stride * m_height;

      in     = (unsigned char *)image_ptr;
      m_data = (char *)         malloc(m_size);
      out    = (uint32_t *)     m_data;

      if (m_data == 0)
         BSG_THROW("Out of memory");

      for (uint32_t y = 0; y < m_height; y++)
      {
         for (uint32_t x = 0; x < m_width; x++)
         {
            uint32_t i = x * 4;

            out[i + 0] = (uint16_t)in[i + 0];
            out[i + 1] = (uint16_t)in[i + 1];
            out[i + 2] = (uint16_t)in[i + 2];
            out[i + 3] = (uint16_t)in[i + 3];
         }

         out += m_width * 4;
         in  += m_width * 4;
      }
   }
   else if (format == Image::eRGBA32F)
   {
      float          *out;
      unsigned char  *in;

      stride = m_width * 16;
      m_size = stride * m_height;

      in     = (unsigned char *)image_ptr;
      m_data = (char *)         malloc(m_size);
      out    = (float *)        m_data;

      if (m_data == 0)
         BSG_THROW("Out of memory");

      for (uint32_t y = 0; y < m_height; y++)
      {
         for (uint32_t x = 0; x < m_width; x++)
         {
            uint32_t i = x * 4;

            out[i + 0] = (float)in[i + 0] / 255.0f;
            out[i + 1] = (float)in[i + 1] / 255.0f;
            out[i + 2] = (float)in[i + 2] / 255.0f;
            out[i + 3] = (float)in[i + 3] / 255.0f;
         }

         out += m_width * 4;
         in  += m_width * 4;
      }
   }
#endif /* BSG_USE_ES3 */
   else
   {
      BSG_THROW("Unsupported target format");
   }
}

ImagePNG::ImagePNG(uint32_t width, uint32_t height, Image::eFormat fmt)
{
   if (fmt != Image::eRGBA8888)
      BSG_THROW("Only eRGBA8888 formats are supported for new ImagePNGs");

   m_width = width;
   m_height = height;
   m_format = Image::Format(fmt);
   m_internalFormat = Image::InternalFormat(fmt);
   m_type = Image::Type(fmt);

   uint32_t stride = m_width * 4 * sizeof(unsigned char);
   m_size = stride * m_height;

   m_data = (char *)malloc(m_size);

   if (m_data == 0)
      BSG_THROW("Out of memory");
}

ImagePNG::~ImagePNG()
{
   if (m_data != 0)
      free(m_data);
}

void ImagePNG::Save(const std::string &fileName)
{
   int y;
   png_structp png_ptr = NULL;
   png_infop info_ptr = NULL;
   FILE *fp;
   static png_bytepp row_pointers = NULL;
   int hasAlpha = 1;

   /* create file */
   fp = fopen(fileName.c_str(), "wb");
   if (!fp)
      BSG_THROW("Failed to open destination file for PNG save");

   try
   {
      /* initialize stuff */
      png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
      if (!png_ptr)
         BSG_THROW("png_create_write_struct failed");

      info_ptr = png_create_info_struct(png_ptr);
      if (!info_ptr)
         BSG_THROW("png_create_info_struct failed");

#if (PNG_LIBPNG_VER >= 10006)
      if (setjmp(png_jmpbuf(png_ptr)))
         BSG_THROW("setjmp() failed");
#else
      if (setjmp(png_ptr->jmpbuf))
         BSG_THROW("setjmp() failed");
#endif

      png_init_io(png_ptr, fp);

      /* write header */
#if (PNG_LIBPNG_VER >= 10006)
      if (setjmp(png_jmpbuf(png_ptr)))
         BSG_THROW("setjmp() failed");
#else
      if (setjmp(png_ptr->jmpbuf))
         BSG_THROW("setjmp() failed");
#endif

      png_set_IHDR(png_ptr, info_ptr, m_width, m_height,
         8, hasAlpha ? PNG_COLOR_TYPE_RGB_ALPHA : PNG_COLOR_TYPE_RGB,
         PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

      png_write_info(png_ptr, info_ptr);

      row_pointers = (png_bytep*)malloc(sizeof(png_bytep) * png_get_image_height(png_ptr, info_ptr));
      if (!row_pointers)
         BSG_THROW("Out of memory");

      /* initialize the row pointers for the image */
      row_pointers[m_height-1] = (png_bytep)m_data;
      for (y=m_height-1; y>0; y--)
         row_pointers[y-1] = (png_bytep)(row_pointers[y] + m_width * 4);

      /* write bytes */
#if (PNG_LIBPNG_VER >= 10006)
      if (setjmp(png_jmpbuf(png_ptr)))
         BSG_THROW("setjmp() failed");
#else
      if (setjmp(png_ptr->jmpbuf))
         BSG_THROW("setjmp() failed");
#endif

      png_write_image(png_ptr, row_pointers);

      /* end write */
#if (PNG_LIBPNG_VER >= 10006)
      if (setjmp(png_jmpbuf(png_ptr)))
         BSG_THROW("setjmp() failed");
#else
      if (setjmp(png_ptr->jmpbuf))
         BSG_THROW("setjmp() failed");
#endif
      png_write_end(png_ptr, NULL);
   }
   catch (Exception e)
   {
      if (row_pointers != NULL)
         free(row_pointers);

      if (png_ptr != NULL)
         png_destroy_write_struct(&png_ptr, &info_ptr);

      if (fp != NULL)
         fclose(fp);

      throw(e);
   }

   if (row_pointers != NULL)
      free(row_pointers);

   if (png_ptr != NULL)
      png_destroy_write_struct(&png_ptr, &info_ptr);

   if (fp != NULL)
      fclose(fp);
}

}

#endif /* BSG_STAND_ALONE */
