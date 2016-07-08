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

#ifndef __BSG_IMAGE_H__
#define __BSG_IMAGE_H__

#include "bsg_glapi.h"

#include <string>
#include <stdint.h>
#include <vector>
#include "bsg_common.h"
#include "bsg_no_copy.h"

namespace bsg
{

class ImageImpl;

//! The BSG supports loading of "png", "pkm" (ETC format) and "raw" image data.
//!
//! The Image class provides a generic interface to image data loaded from disk.  The constructors use the file extension to decide what
//! the format of the requested data is.  It is, in effect, a virtual constructor.  So for example:
//! \code
//! Image image("Earth.png", Image::eRGB888)
//! \endcode
//! will create an image from the file "Earth.png" and convert it to RGB888 format and:
//! \code
//! Image image("Earth.pkm")
//! \endcode
//! will load an ETC texture.
//!
//! The image data will be deleted when the object goes out of scope.
//!
//! The usual use for the Image data is to create an OpenGL texture.  For example:
//! \code
//! texture->TexImage2D(Image("Earth.png", Image::eRGB888));
//! \endcode
//! where "texture" is a bsg::GLTexture will load an image from disk and submit it as a GL texture.
//! The image data will be deleted at the end of the statement as the temporary object goes out of scope.
//!
//! For loading textures with mipmaps, see bsg::ImageSet.
class Image : public NoCopy
{
public:
   enum eFormat
   {
      eRGB565     ,
      eRGBA5551   ,
      eRGBA4444   ,
      eRGB888     ,
      eRGBA8888   ,
      eA8         ,
      eL8         ,
      eLA88       ,
      eETC1       ,
#ifdef BSG_USE_ES3
      eETC2       ,
      eRGBA16F    ,
      eRGBA16I    ,
      eRGBA16UI   ,
      eRGBA32F    ,
      eRGBA32I    ,
      eRGBA32UI   ,
#endif
      eASTC       ,
      eNONE
   };

   //! Image constructor taking a base filename plus an explicit extension name  (explicit width and height for raw formats).
   Image(const std::string &basename, const std::string &ext, eFormat fmt = eNONE);
   Image(const std::string &basename, const std::string &ext, eFormat fmt, uint32_t width, uint32_t height);

   //! Image constructor taking a full filename (explicit width and height for raw formats).
   Image(const std::string &filename, eFormat fmt = eNONE);
   Image(const std::string &filename, eFormat fmt, uint32_t width, uint32_t height);

   //! Deletes an image object and all the underlying image data
   ~Image();

   //! Accessor methods provide details of image dimensions and format
   GLenum      GetInternalFormat()      const;
#ifdef BSG_USE_ES3
   GLenum      GetSizedInternalFormat() const;
#endif
   GLuint      GetWidth()               const;
   GLuint      GetHeight()              const;
   GLenum      GetFormat()              const;
   GLenum      GetType()                const;
   uint32_t    GetSize()                const;
   bool        IsCompressed()           const;

   //! Access the raw memory holding the image data
   const void  *GetData()               const;

   //! Some general helper functions for converting image enums to GL enums
   static GLenum     InternalFormat(eFormat f);
   static GLenum     Format(eFormat f);
   static GLenum     Type(eFormat f);
   static uint32_t   Sizeof(eFormat f);

private:
   void Construct(const std::string &basename, const std::string &ext, eFormat fmt = eNONE);
   void Construct(const std::string &basename, const std::string &ext, eFormat fmt, uint32_t width, uint32_t height);

private:
   ImageImpl   *m_impl;
};

// @cond
// The internal implementation of images
class ImageImpl
{
public:
   virtual ~ImageImpl() {}

   virtual GLenum      GetInternalFormat() const  = 0;
   virtual GLuint      GetWidth()          const  = 0;
   virtual GLuint      GetHeight()         const  = 0;
   virtual GLenum      GetFormat()         const  = 0;
   virtual GLenum      GetType()           const  = 0;
   virtual uint32_t    GetSize()           const  = 0;
   virtual bool        IsCompressed()      const  = 0;
   virtual const void  *GetData()          const  = 0;
};
// @endcond

inline GLenum      Image::GetInternalFormat() const  { return m_impl->GetInternalFormat();  }
inline GLuint      Image::GetWidth()          const  { return m_impl->GetWidth();           }
inline GLuint      Image::GetHeight()         const  { return m_impl->GetHeight();          }
inline GLenum      Image::GetFormat()         const  { return m_impl->GetFormat();          }
inline GLenum      Image::GetType()           const  { return m_impl->GetType();            }
inline uint32_t    Image::GetSize()           const  { return m_impl->GetSize();            }
inline bool        Image::IsCompressed()      const  { return m_impl->IsCompressed();       }

inline const void  *Image::GetData()          const  { return m_impl->GetData();            }

//! The ImageSet class is similar to bsg::Image except that it loads in a series of images for use as a mipmap pyramid.
//!
//! The ImageSet constructors take a base filename and an extension.  The number of levels can either be explicity set, or
//! the constructor can work it out.
//!
//! Images for loading into an ImageSet use the naming convention: basenameN.extension where N is the level of the texture
//! starting at 0.
//!
//! ImageSet is used in a similar way to image, so for example:
//! \code
//! texture->SetAutoMipmap(false);
//! texture->TexImage2D(ImageSet("earth_land", "pkm", 10, Image::eETC1));
//! \endcode
//! will load a set of 10 images into an image set and submit them into a texture object.
class ImageSet : public NoCopy
{
public:
   //! Load "levels" images into an ImageSet.
   ImageSet(const std::string &basename, const std::string &ext, uint32_t levels, Image::eFormat fmt);
   ImageSet(const std::string &basename, const std::string &ext, uint32_t levels, Image::eFormat fmt, uint32_t width, uint32_t height);
   ImageSet(const std::string &basename, const std::string &ext, uint32_t levels, uint32_t baseLevel, uint32_t maxLevel, Image::eFormat fmt);

   //! Load images into a LevelSet.  The number of levels is automatically determined according to what files are present.
   ImageSet(const std::string &basename, const std::string &ext, Image::eFormat fmt);
   ImageSet(const std::string &basename, const std::string &ext, Image::eFormat fmt, uint32_t width, uint32_t height);

   //! Deletes all the bsg::Image objects in the LevelSet.
   ~ImageSet();

   //! Returns the number of levels.
   uint32_t    NumLevels() const   { return m_images->size(); }
   uint32_t    BaseLevel() const   { return m_baseLevel;      }
   uint32_t    MaxLevel()  const   { return m_maxLevel;       }

   //! Returns a reference to image "n".
   const Image *GetLevel(uint32_t n) const   { return (*m_images)[n];  }

private:
   enum
   {
      MAX_LEVELS = 12
   };

private:
   void Initialise(const std::string &basename, const std::string &extension, uint32_t levels, uint32_t baseLevel, uint32_t maxLevel, Image::eFormat fmt);
   void Initialise(const std::string &basename, const std::string &extension, uint32_t levels, uint32_t baseLevel, uint32_t maxLevel, Image::eFormat fmt, uint32_t width, uint32_t height);

private:
   std::vector<Image *>  *m_images;
   uint32_t               m_baseLevel;
   uint32_t               m_maxLevel;
};

#ifdef BSG_USE_ES3

// Wrapper for 3D textures (experimental)
class ImageArray : public NoCopy
{
public:
   ImageArray(const std::string &filename, uint32_t width, uint32_t height, uint32_t slices,
                                           uint32_t maxVal, uint32_t bytesPerPixel, bool bigEndian);
   ImageArray(const std::string &filename, uint32_t width, uint32_t height, uint32_t slices);
   virtual ~ImageArray();

   //! Accessor methods provide details of image dimensions and format
   GLenum      GetInternalFormat()  const { return GL_LUMINANCE;                  }
   GLuint      GetWidth()           const { return m_width;                       }
   GLuint      GetHeight()          const { return m_height;                      }
   GLuint      GetSlices()          const { return m_slices;                      }
   GLenum      GetFormat()          const { return GL_LUMINANCE;                  }
   GLenum      GetType()            const { return GL_UNSIGNED_BYTE;              }
   uint32_t    GetSize()            const { return m_width * m_height * m_slices; }

   //! Access the raw memory holding the image data
   const void  *GetData()           const { return m_data;                        }

private:
   void Construct(const std::string &filename, uint32_t width, uint32_t height, uint32_t slices,
                                               uint32_t maxVal, uint32_t bytesPerPixel, bool bigEndian);

private:
   uint32_t    m_width;
   uint32_t    m_height;
   uint32_t    m_slices;
   uint8_t     *m_data;
};

#endif

}

#endif // __BSG_IMAGE_H__

