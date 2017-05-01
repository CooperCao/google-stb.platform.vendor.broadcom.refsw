/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef BSG_STAND_ALONE

#ifndef __BSG_IMAGE_PNG_H__
#define __BSG_IMAGE_PNG_H__

#include "bsg_common.h"
#include "bsg_image.h"

namespace bsg
{

// @cond
class ImagePNG : public ImageImpl
{
public:
   ImagePNG(const std::string &fileName, Image::eFormat fmt);

   ImagePNG(uint32_t width, uint32_t height, Image::eFormat fmt);

   //! Only images in RGBA8888 format are supported by Save() right now
   void Save(const std::string &fileName);

   virtual ~ImagePNG();

   virtual GLenum      GetInternalFormat() const  { return m_internalFormat;    }
   virtual GLuint      GetWidth()          const  { return m_width;             }
   virtual GLuint      GetHeight()         const  { return m_height;            }
   virtual GLenum      GetFormat()         const  { return m_format;            }
   virtual GLenum      GetType()           const  { return m_type;              }
   virtual uint32_t    GetSize()           const  { return m_size;              }
   virtual bool        IsCompressed()      const  { return false;               }

   const void  *GetData()                  const  { return m_data;              }

private:
   GLenum   m_internalFormat;
   GLuint   m_width;
   GLuint   m_height;
   GLenum   m_format;
   GLenum   m_type;
   uint32_t m_size;
   void     *m_data;
};
// @endcond

}

#endif // __BSG_IMAGE_PNG_H__

#endif /* BSG_STAND ALONE */
