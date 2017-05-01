/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef __BSG_IMAGE_RAW_H__
#define __BSG_IMAGE_RAW_H__

#include "bsg_glapi.h"

#include "bsg_common.h"
#include "bsg_image.h"

namespace bsg
{

// @cond
class ImageRAW : public ImageImpl
{
public:
   ImageRAW(const std::string &fileName, Image::eFormat format, uint32_t width, uint32_t height);

   virtual ~ImageRAW();

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
   GLenum   m_format;
   GLuint   m_width;
   GLuint   m_height;
   GLenum   m_type;
   uint32_t m_size;
   void     *m_data;
};
// @endcond

}

#endif // __BSG_IMAGE_RAW_H__
