/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include <malloc.h>
#include <assert.h>

#include "bsg_image.h"
#include "bsg_image_png.h"
#include "bsg_image_pkm.h"
#include "bsg_image_astc.h"
#include "bsg_image_raw.h"
#include "bsg_exception.h"
#include "bsg_parse_utils.h"  // For ToLower and IsFile
#include "bsg_application.h"

#include <GLES2/gl2ext.h>

#include <stdint.h>
#include <sstream>

using namespace std;

namespace bsg
{

GLenum Image::InternalFormat(eFormat f)
{
   GLenum   fmt = GL_FALSE;

#ifdef BSG_USE_ES3
   switch (f)
   {
   case eRGB565   : fmt = GL_RGB565;               break;
   case eRGB888   : fmt = GL_RGB8;                 break;
   case eRGBA4444 : fmt = GL_RGBA4;                break;
   case eRGBA5551 : fmt = GL_RGB5_A1;              break;
   case eRGBA8888 : fmt = GL_RGBA8;                break;
   case eETC1     : fmt = GL_ETC1_RGB8_OES;        break;
   case eETC2     : fmt = GL_COMPRESSED_RGB8_ETC2; break;
   case eL8       : fmt = GL_LUMINANCE;            break;
   case eA8       : fmt = GL_ALPHA;                break;
   case eLA88     : fmt = GL_LUMINANCE_ALPHA;      break;
   case eR16F     : fmt = GL_R16F;                 break;
   case eR16I     : fmt = GL_R16I;                 break;
   case eR16UI    : fmt = GL_R16UI;                break;
   case eRG16F    : fmt = GL_RG16F;                break;
   case eRG16I    : fmt = GL_RG16I;                break;
   case eRG16UI   : fmt = GL_RG16UI;               break;
   case eRGBA16F  : fmt = GL_RGBA16F;              break;
   case eRGBA16I  : fmt = GL_RGBA16I;              break;
   case eRGBA16UI : fmt = GL_RGBA16UI;             break;
   case eRGBA32F  : fmt = GL_RGBA32F;              break;
   case eRGBA32I  : fmt = GL_RGBA32I;              break;
   case eRGBA32UI : fmt = GL_RGBA32UI;             break;
   case eASTC     : fmt = GL_RGBA;                 break;
   case eNONE     : break;
   }
#else
   switch (f)
   {
   case eRGB565   : fmt = GL_RGB;             break;
   case eRGB888   : fmt = GL_RGB;             break;
   case eRGBA4444 : fmt = GL_RGBA;            break;
   case eRGBA5551 : fmt = GL_RGBA;            break;
   case eRGBA8888 : fmt = GL_RGBA;            break;
   case eETC1     : fmt = GL_ETC1_RGB8_OES;   break;
   case eL8       : fmt = GL_LUMINANCE;       break;
   case eA8       : fmt = GL_ALPHA;           break;
   case eLA88     : fmt = GL_LUMINANCE_ALPHA; break;
   case eASTC     : fmt = GL_RGBA;            break;
   case eNONE     : break;
   }
#endif

   return fmt;
}

GLenum Image::Format(eFormat f)
{
   GLenum   fmt = GL_FALSE;

   switch (f)
   {
   case eRGB565   : fmt = GL_RGB;               break;
   case eRGB888   : fmt = GL_RGB;               break;
   case eRGBA4444 : fmt = GL_RGBA;              break;
   case eRGBA5551 : fmt = GL_RGBA;              break;
   case eRGBA8888 : fmt = GL_RGBA;              break;
   case eL8       : fmt = GL_LUMINANCE;         break;
   case eA8       : fmt = GL_ALPHA;             break;
   case eLA88     : fmt = GL_LUMINANCE_ALPHA;   break;
   case eASTC     : fmt = GL_RGBA;              break;
   case eETC1     : break;
   case eNONE     : break;

#ifdef BSG_USE_ES3
   case eR16F     : fmt = GL_RED;               break;
   case eR16I     : fmt = GL_RED_INTEGER;       break;
   case eR16UI    : fmt = GL_RED_INTEGER;       break;
   case eRG16F    : fmt = GL_RG;                break;
   case eRG16I    : fmt = GL_RG_INTEGER;        break;
   case eRG16UI   : fmt = GL_RG_INTEGER;        break;
   case eRGBA16F  : fmt = GL_RGBA;              break;
   case eRGBA32F  : fmt = GL_RGBA;              break;
   case eRGBA16I  : fmt = GL_RGBA_INTEGER;      break;
   case eRGBA16UI : fmt = GL_RGBA_INTEGER;      break;
   case eRGBA32I  : fmt = GL_RGBA_INTEGER;      break;
   case eRGBA32UI : fmt = GL_RGBA_INTEGER;      break;
   case eETC2     : break;
#endif
   }

   return fmt;
}

GLenum Image::Type(Image::eFormat f)
{
   GLenum   type = GL_FALSE;

   switch (f)
   {
   case eRGBA8888 : type = GL_UNSIGNED_BYTE;          break;
   case eRGB888   : type = GL_UNSIGNED_BYTE;          break;
   case eA8       : type = GL_UNSIGNED_BYTE;          break;
   case eL8       : type = GL_UNSIGNED_BYTE;          break;
   case eLA88     : type = GL_UNSIGNED_BYTE;          break;
   case eRGB565   : type = GL_UNSIGNED_SHORT_5_6_5;   break;
   case eRGBA4444 : type = GL_UNSIGNED_SHORT_4_4_4_4; break;
   case eRGBA5551 : type = GL_UNSIGNED_SHORT_5_5_5_1; break;
   case eASTC     : type = GL_UNSIGNED_BYTE;          break;
   case eETC1     : break;
   case eNONE     : break;

#ifdef BSG_USE_ES3
   case eR16F     : type = GL_HALF_FLOAT;             break;
   case eR16I     : type = GL_SHORT;                  break;
   case eR16UI    : type = GL_UNSIGNED_SHORT;         break;
   case eRG16F    : type = GL_HALF_FLOAT;             break;
   case eRG16I    : type = GL_SHORT;                  break;
   case eRG16UI   : type = GL_UNSIGNED_SHORT;         break;
   case eRGBA16F  : type = GL_HALF_FLOAT;             break;
   case eRGBA16I  : type = GL_SHORT;                  break;
   case eRGBA16UI : type = GL_UNSIGNED_SHORT;         break;
   case eRGBA32F  : type = GL_FLOAT;                  break;
   case eRGBA32I  : type = GL_INT;                    break;
   case eRGBA32UI : type = GL_UNSIGNED_INT;           break;
   case eETC2     : break;
#endif
   }

   return type;
}

uint32_t Image::Sizeof(eFormat f)
{
   uint32_t   size = 0;

   switch (f)
   {
   case eETC1     : size = 4;       break;
   case eL8       : size = 8;       break;
   case eA8       : size = 8;       break;
   case eRGB565   : size = 16;      break;
   case eRGBA5551 : size = 16;      break;
   case eRGBA4444 : size = 16;      break;
   case eLA88     : size = 16;      break;
   case eRGB888   : size = 24;      break;
   case eRGBA8888 : size = 32;      break;
   case eASTC     : size = 32;      break;
   case eNONE     : break;

#ifdef BSG_USE_ES3
   case eR16F     : size = 16;      break;
   case eR16I     : size = 16;      break;
   case eR16UI    : size = 16;      break;
   case eRG16F    : size = 32;      break;
   case eRG16I    : size = 32;      break;
   case eRG16UI   : size = 32;      break;
   case eRGBA16F  : size = 64;      break;
   case eRGBA16I  : size = 64;      break;
   case eRGBA16UI : size = 64;      break;
   case eRGBA32F  : size = 128;     break;
   case eRGBA32I  : size = 128;     break;
   case eRGBA32UI : size = 128;     break;
   case eETC2     : break;
#endif
   }

   return size;
}

//! A factory for images
void Image::Construct(const std::string &basename, const string &ext, eFormat fmt)
{
   string   fileToFind = basename + "." + ext;
   string   filename   = Application::Instance()->FindResource(fileToFind);
   string   extLower   = ParseUtils::ToLower(ext);

   if (filename == "")
      BSG_THROW("Couldn't find " << fileToFind);

#ifndef BSG_STAND_ALONE
   if (extLower == "png")
   {
      m_impl = new ImagePNG(filename, fmt);
      return;
   }
#endif

   if (extLower == "pkm")
   {
      m_impl = new ImagePKM(filename);
      return;
   }

   if (extLower == "astc")
   {
      m_impl = new ImageASTC(filename);
      return;
   }

   BSG_THROW("Unsupported extension '" << ext << "' for image");
}

void Image::Construct(const std::string &basename, const string &ext, eFormat fmt, uint32_t width, uint32_t height)
{
   string   filename = Application::Instance()->FindResource(basename + "." + ext);
   string   extLower = ParseUtils::ToLower(ext);

   if (extLower == "raw")
   {
      m_impl = new ImageRAW(filename, fmt, width, height);
      return;
   }

   BSG_THROW("Unsupported extension '" << ext << "' for image");
}

Image::Image(const std::string &basename, const string &ext, eFormat fmt)
{
   Construct(basename, ext, fmt);
}

Image::Image(const std::string &basename, const string &ext, eFormat fmt, uint32_t width, uint32_t height)
{
   Construct(basename, ext, fmt, width, height);
}

Image::Image(const std::string &filename, eFormat fmt)
{
   int      idx = filename.find_last_of('.');

   string   basename = filename.substr(0, idx);
   string   ext = filename.substr(idx + 1);

   Construct(basename, ext, fmt);
}

Image::Image(const std::string &filename, eFormat fmt, uint32_t width, uint32_t height)
{
   int      idx = filename.find_last_of('.');

   string   basename = filename.substr(0, idx);
   string   ext = filename.substr(idx + 1);

   Construct(basename, ext, fmt, width, height);
}

Image::~Image()
{
   delete m_impl;
}

void ImageSet::Initialise(const string &basename, const string &ext, uint32_t levels, uint32_t baseLevel, uint32_t maxLevel, Image::eFormat fmt)
{
   m_images = new vector<Image *>(levels);

   m_baseLevel = baseLevel;
   m_maxLevel  = maxLevel;

   for (uint32_t l = baseLevel; l <= maxLevel; ++l)
   {
      stringstream   ss;
      ss << l;

      (*m_images)[l] = new Image(basename + ss.str(), ext, fmt);
   }
}

void ImageSet::Initialise(const string &basename, const string &ext, uint32_t levels, uint32_t baseLevel, uint32_t maxLevel, Image::eFormat fmt, uint32_t width, uint32_t height)
{
   m_images = new vector<Image *>(levels);

   m_baseLevel = baseLevel;
   m_maxLevel  = maxLevel;

   for (uint32_t l = 0; l < levels; ++l)
   {
      stringstream   ss;
      ss << l;

      if (l >= baseLevel && l <= maxLevel)
         (*m_images)[l] = new Image(basename + ss.str(), ext, fmt, width, height);

      width = width / 2;
      if (width < 1)
         width = 1;

      height = height / 2;
      if (height < 1)
         height = 1;
   }
}

ImageSet::ImageSet(const string &basename, const string &ext, Image::eFormat fmt)
{
   int      level;
   string   filename;

   for (level = 0; level < MAX_LEVELS; ++level)
   {
      stringstream   ss;
      ss << level;

      filename = basename + ss.str() + "." + ext;

      if (!ParseUtils::IsFile(Application::Instance()->FindResource(filename)))
         break;
   }

   if (level == 0)
      BSG_THROW("No image files found (looking for " << filename << ")");

   Initialise(basename, ext, level, 0, level - 1, fmt);
}

ImageSet::ImageSet(const string &basename, const string &ext, uint32_t levels, Image::eFormat fmt)
{
   Initialise(basename, ext, levels, 0, levels - 1, fmt);
}

ImageSet::ImageSet(const string &basename, const string &ext, uint32_t levels, uint32_t baseLevel, uint32_t maxLevel, Image::eFormat fmt)
{
   Initialise(basename, ext, levels, baseLevel, maxLevel, fmt);
}

ImageSet::ImageSet(const string &basename, const string &ext, Image::eFormat fmt, uint32_t /*width*/, uint32_t /*height*/)
{
   int      level;
   string   filename;

   for (level = 0; level < MAX_LEVELS; ++level)
   {
      stringstream   ss;
      ss << level;

      filename = basename + ss.str() + "." + ext;

      if (!ParseUtils::IsFile(Application::Instance()->FindResource(filename)))
         break;
   }

   if (level == 0)
      BSG_THROW("No image files found (looking for " << filename << ")");

   Initialise(basename, ext, level, 0, level - 1, fmt);
}

ImageSet::ImageSet(const string &basename, const string &ext, uint32_t levels, Image::eFormat fmt, uint32_t width, uint32_t height) :
   m_images(new vector<Image *>(levels))
{
   Initialise(basename, ext, levels, 0, levels - 1, fmt, width, height);
}

ImageSet::~ImageSet()
{
   for (uint32_t i = 0; i < m_images->size(); ++i)
      delete (*m_images)[i];

   delete m_images;
}

#ifdef BSG_USE_ES3

void ImageArray::Construct(const std::string &filename, uint32_t width, uint32_t height, uint32_t slices,
                                                        uint32_t maxVal, uint32_t bytesPerPixel, bool bigEndian)
{
   m_width  = width;
   m_height = height;
   m_slices = slices;
   m_data   = NULL;

   if (bytesPerPixel > 4)
      BSG_THROW("ImageArray: only supports up to 4 bytes per pixel");

   uint32_t   numElems = width * height * slices;
   uint32_t   size     = numElems * bytesPerPixel;

   std::string file = Application::Instance()->FindResource(filename);

   FILE  *fp = fopen(file.c_str(), "rb");

   if (fp == NULL)
      BSG_THROW("ImageArray: Can't open file " << file);

   std::vector<uint8_t>   temp(size);

   m_data = new uint8_t[numElems];

   if (m_data == NULL)
      BSG_THROW("ImageArray: Out of memory");

   fread(&temp[0], size, 1, fp);

   uint8_t *ptr = &temp[0];

   for (uint32_t i = 0; i < numElems; ++i, ptr += bytesPerPixel)
   {
      uint32_t datum = 0;

      for (uint32_t b = 0; b < bytesPerPixel; ++b)
      {
         uint32_t   ix = bigEndian ? b : bytesPerPixel - b - 1;
         datum = (datum << 8) | ptr[ix];
      }

      m_data[i] = (uint8_t)(((float)datum / maxVal) * 255.0);
   }
}

ImageArray::ImageArray(const std::string &filename, uint32_t width, uint32_t height, uint32_t slices,
                                                    uint32_t maxVal, uint32_t bytesPerPixel, bool bigEndian)
{
   Construct(filename, width, height, slices, maxVal, bytesPerPixel, bigEndian);
}

ImageArray::ImageArray(const std::string &filename, uint32_t width, uint32_t height, uint32_t slices)
{
   Construct(filename, width, height, slices, 4095, 2, true/*bigEndian*/);
}

ImageArray::~ImageArray()
{
   delete [] m_data;
}

#endif

}
