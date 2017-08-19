/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef __BSG_GL_TEXTURE_H__
#define __BSG_GL_TEXTURE_H__

#include "bsg_glapi.h"

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include "bsg_common.h"
#include "bsg_image.h"
#include "bsg_cube_images.h"
#include "bsg_library.h"
#include "bsg_gl_program.h"
#include "bsg_animatable.h"
#include "bsg_no_copy.h"
#include "bsg_task.h"

namespace bsg
{

class GLTexture;
class NativePixmap;

// @cond
struct GLTextureTraits
{
   typedef GLTexture       Base;
   typedef GLTexture       Derived;
   typedef GLTextureTraits BaseTraits;
};
// @endcond

//! @addtogroup handles
//! @{
typedef Handle<GLTextureTraits>  TextureHandle;
//! @}

//! This bsg::Task is useful for loading textures on a secondary thread using .
//! Note that the work of loading data from files is done in the worker thread where it may take some time (e.g. if
//! the data is obtained from a slow network.  However, submitting the texture data to GL is done in the OnCallback()
//! method which runs in the main thread (which has the GL context).
//! A Worker must supply
//! - a typedef for its ImageType
//! - a Load() method expected to return a pointer to a new (heap allocated) image
//! - a TexImage() method taking the texture handle and reference to the image
//!
//! See the bsg::TexImage2D and bsg::TexImageCube workers which are examples of the required interface.
template <class Worker>
class TexImageTask : public Task
{
public:
   TexImageTask(GLTexture *texture, const Worker &worker) :
      m_texture(texture),
      m_worker(worker)
   {}

   virtual void OnThread()
   {
      m_image = std::unique_ptr<typename Worker::ImageType>(m_worker.Load());
   }

   virtual void OnCallback(bool finished)
   {
      m_worker.TexImage(m_texture, *m_image);
   }

private:
   GLTexture                                 *m_texture;
   std::unique_ptr<typename Worker::ImageType> m_image;
   Worker                                    m_worker;
};


//! The BSG provides wrappers around GL texture objects which make them easier to create and use.
//!
//! Textures can be either 2D textures or cubemap textures, and may be mipmapped.
//!
//! Textures are normally used by bsg::Material thus:
//! \code
//! material->SetTexture("u_sampler", texture);
//! \endcode
//!
//! which connects the texture to the uniform sampler name "u_sampler" which in turn will be defined
//! in the effect file loaded into a bsg::Effect.
//!
//! Textures will not be automatically mipmapped, unless you call SetAutoMipmap() prior to calling any of the TexImage2D
//! methods.  Auto-mipmap is costly so should be avoided when submitting textures rapidly.
//!
//! For best rendering performance, you should normally use mipmapped textures.
//!
//! Auto-mipmapping is not currently supported for ETC textures, you should instead use ImageSet to load
//! pre-calculated mipmap levels.
class GLTexture : public RefCount, public NoCopy
{
   friend class Handle<GLTextureTraits>;

public:
   enum eVideoTextureMode
   {
      TEX_IMAGE_2D         = 0,             //!< \deprecated Use eTEX_IMAGE_2D
      EGL_IMAGE,                            //!< \deprecated Use eEGL_IMAGE

      eTEX_IMAGE_2D        = TEX_IMAGE_2D,  //!< Use glTexImage2D to re-submit each frame of video.
      eEGL_IMAGE           = EGL_IMAGE,     //!< Use EGLImage. The video data will update automatically.
      eEGL_IMAGE_EXPLICIT                   //!< Use EGLImage with explicit update region calls.
   };

public:
   //! Deletes the GL texture.
   virtual ~GLTexture();

   //! Have images been uploaded to the texture.
   bool HasData() const
   {
      return m_hasData;
   }

   //! Binds the GL texture to the specified target.
   void Bind(GLenum target) const
   {
      glBindTexture(target, m_id);
   }

   //! Sets the automipmap flag.  When an image is bound to the texture, this flag controls whether
   //! a mipmap chain is automatically generated.  Do not set automipmap for ETC/PKM textures.
   void SetAutoMipmap(bool genMipmap) { m_genMipmap = genMipmap; }

   //! Submit a single bsg::Image as a level of the 2D texture.
   void TexImage2D(GLuint level, const Image &image)
   {
      uint32_t w = image.GetWidth();
      uint32_t h = image.GetHeight();

      ClearEGLImage();

      Bind(GL_TEXTURE_2D);

      if (!image.IsCompressed())
      {
         glTexImage2D(GL_TEXTURE_2D, level, image.GetInternalFormat(), w, h, 0, image.GetFormat(), image.GetType(), image.GetData());
         if (m_genMipmap && level == 0)
            glGenerateMipmap(GL_TEXTURE_2D);
      }
      else
      {
         glCompressedTexImage2D(GL_TEXTURE_2D, level, image.GetInternalFormat(), w, h, 0, image.GetSize(), image.GetData());
         if (m_genMipmap)
            BSG_THROW("Cannot generate mipmaps for compressed images\n");
      }

      m_hasData = true;
   }

   //! Submit a single bsg::Image as level zero of the 2D texture.
   void TexImage2D(const Image &image)
   {
      TexImage2D(0, image);
   }

   //! Submit a bsg::ImageSet into the 2D texture levels.
   void TexImage2D(const ImageSet &imageSet)
   {
      uint32_t    numLevels = imageSet.NumLevels();

      // Mipmap has been specified explicitly, so don't try to generate
      m_genMipmap = false;

#ifdef BSG_USE_ES3
      const Image *image    = imageSet.GetLevel(imageSet.BaseLevel());

      Bind(GL_TEXTURE_2D);

      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, imageSet.BaseLevel());
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL,  imageSet.MaxLevel());

      if (image && !image->IsCompressed())
      {
         glTexStorage2D(GL_TEXTURE_2D, numLevels, image->GetInternalFormat(), image->GetWidth(), image->GetHeight());

         for (uint32_t l = imageSet.BaseLevel(); l <= imageSet.MaxLevel(); ++l)
         {
            const Image *level = imageSet.GetLevel(l);

            if (level)
            {
               glTexSubImage2D(GL_TEXTURE_2D, l, 0, 0, level->GetWidth(), level->GetHeight(), level->GetFormat(), level->GetType(), level->GetData());
            }
         }
      }
      else
      {
         for (uint32_t l = imageSet.BaseLevel(); l <= imageSet.MaxLevel(); ++l)
         {
            const Image *level = imageSet.GetLevel(l);

            if (level)
               TexImage2D(l, *level);
         }
      }
#else
      for (uint32_t l = 0; l < numLevels; ++l)
         TexImage2D(l, *imageSet.GetLevel(l));
#endif
   }

   //! Submit raw image data to the 2D texture level.
   void TexImage2D(GLuint level, GLenum internalFormat,
                   GLuint w, GLuint h, GLenum format, GLenum type, const void *data)
   {
      ClearEGLImage();

      Bind(GL_TEXTURE_2D);
      glTexImage2D(GL_TEXTURE_2D, level, internalFormat, w, h, 0, format, type, data);

      if (m_genMipmap && level == 0)
         glGenerateMipmap(GL_TEXTURE_2D);

      m_hasData = true;
   }

   //! Submit raw image data to the 2D texture level zero.
   void TexImage2D(GLenum internalFormat,
                   GLuint w, GLuint h, GLenum format, GLenum type, const void *data)
   {
      TexImage2D(0, internalFormat, w, h, format, type, data);
   }

   //! Submit video frame image data to the 2D texture level zero.
   void TexImage2D(NativePixmap *pixmap, eVideoTextureMode mode = EGL_IMAGE);

   //! Submit bsg::Image data to the cubemap texture level.
   void TexImageCube(GLuint level, const Image &imagePosX, const Image &imageNegX,
                                   const Image &imagePosY, const Image &imageNegY,
                                   const Image &imagePosZ, const Image &imageNegZ)
   {
      // Cube maps need to have the same square dimensions and the same formats
      GLuint   d     = imagePosX.GetWidth();
      GLenum   fmt   = imagePosX.GetFormat();
      GLenum   ifmt  = imagePosX.GetInternalFormat();
      GLenum   type  = imagePosX.GetType();

      ClearEGLImage();

      Bind(GL_TEXTURE_CUBE_MAP);

      if (!imagePosX.IsCompressed())
      {
         glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, level, ifmt, d, d, 0, fmt, type, imagePosX.GetData());
         glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, level, ifmt, d, d, 0, fmt, type, imageNegX.GetData());
         glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, level, ifmt, d, d, 0, fmt, type, imagePosY.GetData());
         glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, level, ifmt, d, d, 0, fmt, type, imageNegY.GetData());
         glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, level, ifmt, d, d, 0, fmt, type, imagePosZ.GetData());
         glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, level, ifmt, d, d, 0, fmt, type, imageNegZ.GetData());

         if (m_genMipmap)
            glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
      }
      else
      {
         glCompressedTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, level, ifmt, d, d, 0, imagePosX.GetSize(), imagePosX.GetData());
         glCompressedTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, level, ifmt, d, d, 0, imageNegX.GetSize(), imageNegX.GetData());
         glCompressedTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, level, ifmt, d, d, 0, imagePosY.GetSize(), imagePosY.GetData());
         glCompressedTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, level, ifmt, d, d, 0, imageNegY.GetSize(), imageNegY.GetData());
         glCompressedTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, level, ifmt, d, d, 0, imagePosZ.GetSize(), imagePosZ.GetData());
         glCompressedTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, level, ifmt, d, d, 0, imageNegZ.GetSize(), imageNegZ.GetData());

         if (m_genMipmap)
            BSG_THROW("Cannot generate mipmaps for compressed images\n");
      }

      m_hasData = true;
   }

   //! Submit the bsg::Image data to the cubemap texture level 0.
   void TexImageCube(const Image &imagePosX, const Image &imageNegX,
                     const Image &imagePosY, const Image &imageNegY,
                     const Image &imagePosZ, const Image &imageNegZ)
   {
      TexImageCube(0, imagePosX, imageNegX, imagePosY, imageNegY, imagePosZ, imageNegZ);
   }

   //! Submit the bsg::CubeImages to the cubemap texture level 0.
   void TexImageCube(const CubeImages &images)
   {
      TexImageCube(0, images.GetPosX(), images.GetNegX(),
                      images.GetPosY(), images.GetNegY(),
                      images.GetPosZ(), images.GetNegZ());
   }

   //! Submit a task to delay load a texture
   //! The Worker class must provide a Load() and a TexImage() method.  The Load() will
   //! run in a thread.  The TexImage() will run on the main thread and can e.g. submit
   //! textures.
   template <class Worker>
   void TexImageTask(Tasker &tasker, const Worker &worker)
   {
      tasker.Submit(new bsg::TexImageTask<Worker>(this, worker));
   }

   //! Attach the texture to the framebuffer
   void FramebufferTexture2D(GLenum attachment, GLenum texTarget, GLuint level) const
   {
      if (texTarget == GL_TEXTURE_2D)
         Bind(GL_TEXTURE_2D);
      else
         Bind(GL_TEXTURE_CUBE_MAP);

      glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, texTarget, m_id, level);
   }

#ifdef GL_EXT_multisampled_render_to_texture
   void FramebufferTexture2DMultisample(GLenum attachment, GLenum texTarget, GLuint level, GLsizei samples) const
      {
         if (texTarget == GL_TEXTURE_2D)
            Bind(GL_TEXTURE_2D);
         else
            BSG_THROW("Multisample FBO is only support for TEXTURE_2D");

         if (m_glFramebufferTexture2DMultisampleEXT != NULL)
            m_glFramebufferTexture2DMultisampleEXT(GL_FRAMEBUFFER, attachment, texTarget, m_id, level, samples);
         else
            BSG_THROW("glFramebufferTexture2DMultisampleEXT not supported");
      }
#endif

   //! Inform GL that a particular region of the texture has changed.
   //! Only relevant for EGLImage based textures (i.e. those constructed with a NativePixmap).
   void SetUpdatedRegion(uint32_t x, uint32_t y, uint32_t width, uint32_t height);

   //! For EGLImage based textures only. Lock the texture data so that the application can
   //! change it. Hold the lock for as small a time as possible to avoid stalling the 3D
   //! rendering.
   void Lock();

   //! For EGLImage based textures only. Unlock the texture data following a lock.
   void Unlock();

   void CopyTexImage(uint32_t x, uint32_t y, uint32_t w, uint32_t h)
   {
      Bind(GL_TEXTURE_2D);
      glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, x, y, w, h, 0);
   }

   void CopyTexSubImage(uint32_t /*offx*/, uint32_t /*offy*/, uint32_t x, uint32_t y, uint32_t w, uint32_t h)
   {
      Bind(GL_TEXTURE_2D);
      glCopyTexSubImage2D(GL_TEXTURE_2D, 0, GL_RGB, x, y, w, h, 0);
   }

#ifdef BSG_USE_ES3
   // TODO common up this code.
   void TexImage3D(const ImageArray &volume)
   {
      uint32_t w = volume.GetWidth();
      uint32_t h = volume.GetHeight();
      uint32_t s = volume.GetSlices();
      GLint num_skip_images = 0;

      glGetIntegerv(GL_UNPACK_SKIP_IMAGES, &num_skip_images);

      Bind(GL_TEXTURE_3D);

      glTexImage3D(GL_TEXTURE_3D, 0, volume.GetInternalFormat(), w, h, (s - num_skip_images < 0) ? 0 : s - num_skip_images, 0, volume.GetFormat(), volume.GetType(), volume.GetData());

      if (m_genMipmap)
         glGenerateMipmap(GL_TEXTURE_3D);

      m_hasData = true;
   }

   void TexImage2DArray(const ImageArray &volume)
   {
      uint32_t w = volume.GetWidth();
      uint32_t h = volume.GetHeight();
      uint32_t s = volume.GetSlices();

      Bind(GL_TEXTURE_2D_ARRAY);

      glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, volume.GetInternalFormat(), w, h, s, 0, volume.GetFormat(), volume.GetType(), volume.GetData());

      if (m_genMipmap)
         glGenerateMipmap(GL_TEXTURE_2D_ARRAY);

      m_hasData = true;
   }

#endif

protected:
   //! Create a GL texture object.  By default the texture is considered to be not mipmapped.
   GLTexture() :
      m_hasData(false),
      m_genMipmap(false),
      m_currentPixmap(NULL),
      m_eglImage(NULL)
   {
      InitExtensions();
      glGenTextures(1, &m_id);
   }

   void ClearEGLImage();

   static void InitExtensions();

private:
   GLuint            m_id;
   bool              m_hasData;
   bool              m_genMipmap;
   NativePixmap      *m_currentPixmap;
   EGLImageKHR       m_eglImage;

   static bool                                m_extensionsInited;
   static PFNEGLCREATEIMAGEKHRPROC            m_eglCreateImageKHR;
   static PFNEGLDESTROYIMAGEKHRPROC           m_eglDestroyImageKHR;
   static PFNGLEGLIMAGETARGETTEXTURE2DOESPROC m_glEGLImageTargetTexture2DOES;
#ifdef EGL_BRCM_image_update_control
   static PFNEGLIMAGEUPDATEPARAMETERIVBRCMPROC m_eglImageUpdateParameterivBRCM;
   static PFNEGLIMAGEUPDATEPARAMETERIBRCMPROC  m_eglImageUpdateParameteriBRCM;
#endif
#ifdef GL_EXT_multisampled_render_to_texture
   static PFNGLFRAMEBUFFERTEXTURE2DMULTISAMPLEEXTPROC m_glFramebufferTexture2DMultisampleEXT;
#endif
};

// @cond
class GLSamplerState
{
public:
   GLSamplerState() :
      m_unit(0),
      m_target(GL_TEXTURE_2D),
      m_wrapS(GL_CLAMP_TO_EDGE),
      m_wrapT(GL_CLAMP_TO_EDGE),
      m_minFilter(GL_LINEAR),
      m_magFilter(GL_LINEAR),
#ifndef BSG_USE_ES3
      m_paramsDirty(true),
#endif
      m_prog(0)
   {
      Init();
   }

   GLSamplerState(GLuint unit, GLenum target, GLenum wrapS, GLenum wrapT, GLenum minFilter, GLenum magFilter) :
      m_unit(unit),
      m_target(target),
      m_wrapS(wrapS),
      m_wrapT(wrapT),
      m_minFilter(minFilter),
      m_magFilter(magFilter),
#ifndef BSG_USE_ES3
      m_paramsDirty(true),
#endif
      m_prog(0)
   {
      Init();
   }

   GLuint   GetUnit()      const { return m_unit;        }
   GLenum   GetTarget()    const { return m_target;      }
   GLenum   GetWrapS()     const { return m_wrapS;       }
   GLenum   GetWrapT()     const { return m_wrapT;       }
   GLenum   GetMinFilter() const { return m_minFilter;   }
   GLenum   GetMagFilter() const { return m_magFilter;   }

   void Select(const GLProgram &prog, const std::string &name, TextureHandle t) const;

private:
   void Init();

private:
   GLuint            m_unit;
   GLenum            m_target;
   GLenum            m_wrapS;
   GLenum            m_wrapT;
   GLenum            m_minFilter;
   GLenum            m_magFilter;

#ifdef BSG_USE_ES3
   //GLenum            m_minLOD;
   //GLenum            m_maxLOD;
   //GLenum            m_compareMode;
   //GLenum            m_compateFunc;
   GLuint            m_sampler;
#endif

#ifndef BSG_USE_ES3
   mutable bool            m_paramsDirty;
#endif

   mutable const GLProgram *m_prog;
   mutable TextureHandle   m_texture;
};

class GLTextureBinding : public AnimTarget<TextureHandle>
{
public:
   GLTextureBinding(TextureHandle texture, const GLSamplerState *sampler) :
      m_texture(texture),
      m_sampler(sampler)
   {}

   void Select(const GLProgram &prog, const std::string &name) const
   {
      m_sampler->Select(prog, name, m_texture);
   }

   void SetTexture(TextureHandle texture)
   {
      m_texture = texture;
   }

   virtual void Set(const TextureHandle &value)
   {
      SetTexture(value);
   }

   virtual TextureHandle Get() const
   {
      return m_texture;
   }

private:
   TextureHandle        m_texture;
   const GLSamplerState *m_sampler;
};
// @endcond

//! TexImage2D is a worker suitable for delay loading 2D textures.
class TexImage2D
{
public:
   typedef Image  ImageType;

   TexImage2D(const std::string &file, Image::eFormat fmt) :
      m_file(file),
      m_fmt(fmt)
   {}

   TexImage2D(const std::string &file, const std::string &ext, Image::eFormat fmt) :
      m_file(file),
      m_ext(ext),
      m_fmt(fmt)
   {}

   //! Load a 2D texture from a file
   ImageType *Load() const
   {
      ImageType   *image = 0;

      if (m_ext == "")
         image = new ImageType(m_file, m_fmt);
      else
         image = new ImageType(m_file, m_ext, m_fmt);

      return image;
   }

   //! Submit the texture image to the texture
   void TexImage(GLTexture *texture, const ImageType &image)
   {
      texture->TexImage2D(image);
   }

private:
   std::string    m_file;
   std::string    m_ext;
   Image::eFormat m_fmt;
};

//! TexImageCube is a worker suitable for delay loading cube textures.
class TexImageCube
{
public:
   typedef CubeImages  ImageType;

   TexImageCube(const std::string &base, const std::string &ext, Image::eFormat fmt) :
      m_base(base),
      m_ext(ext),
      m_fmt(fmt)
   {}

   //! Load a cubemap image from a set of files
   ImageType *Load() const
   {
      return new ImageType(m_base, m_ext, m_fmt);
   }

   //! Submit the cubemap image to the texture
   void TexImage(GLTexture *texture, const ImageType &image)
   {
      texture->TexImageCube(image);
   }

private:
   std::string    m_base;
   std::string    m_ext;
   Image::eFormat m_fmt;
};

}

#endif // __BSG_GL_TEXTURE_H__
