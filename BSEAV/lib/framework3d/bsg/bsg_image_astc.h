/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef __BSG_IMAGE_ASTC_H__
#define __BSG_IMAGE_ASTC_H__

#include "bsg_glapi.h"

#include "bsg_common.h"
#include "bsg_image.h"

namespace bsg
{

// @cond
class ImageASTC : public ImageImpl
{
public:
   ImageASTC(const std::string &fileName);

   virtual ~ImageASTC();

   virtual GLenum      GetInternalFormat() const;
   virtual GLuint      GetWidth()          const  { return m_width;             }
   virtual GLuint      GetHeight()         const  { return m_height;            }
   virtual GLuint      GetDepth()         const   { return m_depth;             }
   virtual GLenum      GetFormat()         const  { return m_internalFormat;    }
   virtual GLenum      GetType()           const  { return m_type;              }
   virtual uint32_t    GetSize()           const  { return m_size;              }
   virtual bool        IsCompressed()      const;

   const void  *GetData()                  const  { return m_data;              }

private:
   bool SetInternalFormat(uint8_t blockdim_x, uint8_t blockdim_y, bool signedValues);

   GLuint   m_width;
   GLuint   m_height;
   GLuint   m_depth;
   GLenum   m_internalFormat;
   GLenum   m_type;
   uint32_t m_size;

   void     *m_data;
};
// @endcond

}

#endif // __BSG_IMAGE_ASTC_H__
