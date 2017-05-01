/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef __BSG_IMAGE_PKM_H__
#define __BSG_IMAGE_PKM_H__

#include "bsg_glapi.h"

#include "bsg_common.h"
#include "bsg_image.h"

namespace bsg
{

// @cond
class ImagePKM : public ImageImpl
{
public:
   ImagePKM(const std::string &fileName);

   virtual ~ImagePKM();

   virtual GLenum      GetInternalFormat() const;
   virtual GLuint      GetWidth()          const  { return m_width;             }
   virtual GLuint      GetHeight()         const  { return m_height;            }
   virtual GLenum      GetFormat()         const  { return GL_RGB;              }
   virtual GLenum      GetType()           const  { return m_type;              }
   virtual uint32_t    GetSize()           const  { return m_size;              }
   virtual bool        IsCompressed()      const;

   const void  *GetData()                  const  { return m_data;              }

private:
   GLuint   m_width;
   GLuint   m_height;
   GLenum   m_internalFormat;
   GLenum   m_type;
   uint32_t m_size;
   void     *m_data;
};
// @endcond

}

#endif // __BSG_IMAGE_PKM_H__
