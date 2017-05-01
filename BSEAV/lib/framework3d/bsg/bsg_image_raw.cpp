/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "bsg_image_raw.h"
#include "bsg_exception.h"
#include "bsg_trackers.h"

#include <string.h>

#include <string>
#include <stdlib.h>
#include <malloc.h>
#include <stdio.h>

using namespace std;

namespace bsg
{

ImageRAW::ImageRAW(const string &fileName, Image::eFormat format, uint32_t width, uint32_t height)
{
   m_internalFormat = Image::InternalFormat(format);
   m_format         = Image::Format(format);

   m_width  = width;
   m_height = height;
   m_type   = Image::Type(format);
   m_size   = width * height * Image::Sizeof(format) / 8;

   FILE  *fp = fopen(fileName.c_str(), "rb");

   if (fp == NULL)
      BSG_THROW("Cannot open file " << fileName);

   // Mark fp for closure
   FClose   closeIt(fp);

   m_data = (uint8_t *)malloc(m_size);

   if (m_data == 0)
      BSG_THROW("Out of memory");

   if (fread(m_data, sizeof(uint8_t), m_size, fp) != m_size)
      BSG_THROW("Size of raw file " << fileName << " is not correct");
}

ImageRAW::~ImageRAW()
{
   if (m_data != 0)
      free(m_data);
}

}
