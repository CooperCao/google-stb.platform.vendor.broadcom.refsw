/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "bsg_cube_images.h"
#include "bsg_exception.h"
#include "bsg_image.h"

namespace bsg
{

CubeImages::CubeImages(const std::string &basename, const std::string &extension, Image::eFormat fmt) :
   m_image(new std::vector<Image *>(eNUM_FACES))
{
   (*m_image)[eXPOS] = new Image(basename + "_xpos", extension, fmt);
   (*m_image)[eXNEG] = new Image(basename + "_xneg", extension, fmt);
   (*m_image)[eYPOS] = new Image(basename + "_ypos", extension, fmt);
   (*m_image)[eYNEG] = new Image(basename + "_yneg", extension, fmt);
   (*m_image)[eZPOS] = new Image(basename + "_zpos", extension, fmt);
   (*m_image)[eZNEG] = new Image(basename + "_zneg", extension, fmt);
}

CubeImages::CubeImages(const std::string &basename, const std::string &extension, Image::eFormat fmt, uint32_t width, uint32_t height) :
   m_image(new std::vector<Image *>(eNUM_FACES))
{
   (*m_image)[eXPOS] = new Image(basename + "_xpos", extension, fmt, width, height);
   (*m_image)[eXNEG] = new Image(basename + "_xneg", extension, fmt, width, height);
   (*m_image)[eYPOS] = new Image(basename + "_ypos", extension, fmt, width, height);
   (*m_image)[eYNEG] = new Image(basename + "_yneg", extension, fmt, width, height);
   (*m_image)[eZPOS] = new Image(basename + "_zpos", extension, fmt, width, height);
   (*m_image)[eZNEG] = new Image(basename + "_zneg", extension, fmt, width, height);
}

CubeImages::~CubeImages()
{
   for (uint32_t i = 0; i < eNUM_FACES; ++i)
      delete (*m_image)[i];

   delete m_image;
}

}
