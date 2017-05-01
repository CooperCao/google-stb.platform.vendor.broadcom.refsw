/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef __BSG_CUBE_IMAGES_H__
#define __BSG_CUBE_IMAGES_H__

#include "bsg_common.h"
#include "bsg_image.h"

namespace bsg
{

//! Helper class for loading cube-maps
class CubeImages : public NoCopy
{
private:
   enum eFace
   {
      eXPOS = 0,
      eXNEG,
      eYPOS,
      eYNEG,
      eZPOS,
      eZNEG,
      eNUM_FACES
   };

public:
   //! Load images from file basename_xpos.extension, basename_xneg.extension etc. and convert to the specified format.
   //! Images should be located in one of the application resources folder
   CubeImages(const std::string &basename, const std::string &extension, Image::eFormat fmt);

   //! Load images with no size information following the same rules as CubeImages(base, ext, fmt)
   CubeImages(const std::string &basename, const std::string &extension, Image::eFormat fmt, uint32_t width, uint32_t height);
   ~CubeImages();

   const Image &GetPosX() const { return *(*m_image)[eXPOS]; }
   const Image &GetNegX() const { return *(*m_image)[eXNEG]; }
   const Image &GetPosY() const { return *(*m_image)[eYPOS]; }
   const Image &GetNegY() const { return *(*m_image)[eYNEG]; }
   const Image &GetPosZ() const { return *(*m_image)[eZPOS]; }
   const Image &GetNegZ() const { return *(*m_image)[eZNEG]; }

private:
   std::vector<Image *> *m_image;
};

}

#endif /* __BSG_CUBE_IMAGES_H__ */
