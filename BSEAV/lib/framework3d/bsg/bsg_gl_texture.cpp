/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "bsg_gl_texture.h"
#include "bsg_application.h"

namespace bsg
{

bool                                GLTexture::m_extensionsInited = false;
PFNGLEGLIMAGETARGETTEXTURE2DOESPROC GLTexture::m_glEGLImageTargetTexture2DOES = NULL;
PFNEGLCREATEIMAGEKHRPROC            GLTexture::m_eglCreateImageKHR = NULL;
PFNEGLDESTROYIMAGEKHRPROC           GLTexture::m_eglDestroyImageKHR = NULL;

#if EGL_BRCM_image_update_control
PFNEGLIMAGEUPDATEPARAMETERIVBRCMPROC GLTexture::m_eglImageUpdateParameterivBRCM = NULL;
PFNEGLIMAGEUPDATEPARAMETERIBRCMPROC  GLTexture::m_eglImageUpdateParameteriBRCM = NULL;
#endif

#ifdef GL_EXT_multisampled_render_to_texture
PFNGLFRAMEBUFFERTEXTURE2DMULTISAMPLEEXTPROC GLTexture::m_glFramebufferTexture2DMultisampleEXT = NULL;
#endif

void GLTexture::InitExtensions()
{
   if (!m_extensionsInited)
   {
      m_glEGLImageTargetTexture2DOES = (PFNGLEGLIMAGETARGETTEXTURE2DOESPROC)eglGetProcAddress("glEGLImageTargetTexture2DOES");
      m_eglCreateImageKHR = (PFNEGLCREATEIMAGEKHRPROC)eglGetProcAddress("eglCreateImageKHR");
      m_eglDestroyImageKHR = (PFNEGLDESTROYIMAGEKHRPROC)eglGetProcAddress("eglDestroyImageKHR");

      if (!m_glEGLImageTargetTexture2DOES || !m_eglCreateImageKHR || !m_eglDestroyImageKHR)
         printf("Warning: EGLImage texturing is not supported\n");

#if EGL_BRCM_image_update_control
      m_eglImageUpdateParameterivBRCM = (PFNEGLIMAGEUPDATEPARAMETERIVBRCMPROC)eglGetProcAddress("eglImageUpdateParameterivBRCM");
      m_eglImageUpdateParameteriBRCM  = (PFNEGLIMAGEUPDATEPARAMETERIBRCMPROC)eglGetProcAddress("eglImageUpdateParameteriBRCM");
#endif

#if GL_EXT_multisampled_render_to_texture
      m_glFramebufferTexture2DMultisampleEXT = (PFNGLFRAMEBUFFERTEXTURE2DMULTISAMPLEEXTPROC)eglGetProcAddress("glFramebufferTexture2DMultisampleEXT");
#endif
   }

   m_extensionsInited = true;
}

GLTexture::~GLTexture()
{
   glDeleteTextures(1, &m_id);

   ClearEGLImage();
}

void GLTexture::TexImage2D(NativePixmap *pixmap, eVideoTextureMode mode)
{
   if (pixmap == NULL)
      return;

   Bind(GL_TEXTURE_2D);

   if (mode == eEGL_IMAGE || mode == eEGL_IMAGE_EXPLICIT)
   {
      // Create the EGLImage once. It will update automatically afterwards.
      if (pixmap != m_currentPixmap)
      {
         EGLint attr_list[] = { EGL_NONE };

         m_currentPixmap = pixmap;

         ClearEGLImage();

         // Convert buffer to EGLImage
         if (m_eglCreateImageKHR)
            m_eglImage = m_eglCreateImageKHR(Application::Instance()->GetContext().GetDisplay(),
                                           EGL_NO_CONTEXT, EGL_NATIVE_PIXMAP_KHR,
                                           (EGLClientBuffer)pixmap->EGLPixmap(), attr_list);
         if (m_eglImage == EGL_NO_IMAGE_KHR)
            BSG_THROW("Failed to create EGLImage");

#if EGL_BRCM_image_update_control
         if (mode == eEGL_IMAGE_EXPLICIT && m_eglImageUpdateParameteriBRCM)
         {
            // Inform GL that we will tell it when the EGLimage actually updates
            m_eglImageUpdateParameteriBRCM(Application::Instance()->GetContext().GetDisplay(),
                                    m_eglImage, EGL_IMAGE_UPDATE_CONTROL_SET_MODE_BRCM,
                                    EGL_IMAGE_UPDATE_CONTROL_EXPLICIT_BRCM);
         }
#endif

         if (m_eglImage != NULL && m_glEGLImageTargetTexture2DOES != NULL)
            m_glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, m_eglImage);
     }
   }
   else if (mode == eTEX_IMAGE_2D)
   {
      ClearEGLImage();

      // Use standard GL texture submission
      switch (pixmap->GetFormat())
      {
      case NativePixmap::YUV422_TEXTURE   :
#ifdef GL_APPLE_rgb_422
         glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB_422_APPLE, pixmap->GetWidth(), pixmap->GetHeight(), 0,
                      GL_RGB_422_APPLE, GL_UNSIGNED_SHORT_8_8_APPLE, pixmap->GetPixelDataPtr());
         break;
#else
         BSG_THROW("Emulator does not support GL_RGB_422_APPLE");
         break;
#endif
      case NativePixmap::ABGR8888_TEXTURE :
         glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, pixmap->GetWidth(), pixmap->GetHeight(), 0,
                      GL_RGBA, GL_UNSIGNED_BYTE, pixmap->GetPixelDataPtr());
         break;
      case NativePixmap::RGB565_TEXTURE   :
         glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, pixmap->GetWidth(), pixmap->GetHeight(), 0,
                      GL_RGB, GL_UNSIGNED_SHORT_5_6_5, pixmap->GetPixelDataPtr());
         break;
      case NativePixmap::eYV12_TEXTURE:
         BSG_THROW("YV12 not supported in glTexImage2D");
         break;
      }

      if (m_genMipmap)
         glGenerateMipmap(GL_TEXTURE_2D);
   }
}

void GLTexture::ClearEGLImage()
{
   if (m_eglImage != NULL && m_eglDestroyImageKHR)
      m_eglDestroyImageKHR(Application::Instance()->GetContext().GetDisplay(), m_eglImage);

   m_eglImage = NULL;
}

void GLTexture::SetUpdatedRegion(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
{
#if EGL_BRCM_image_update_control
   if (m_eglImage != NULL && m_eglImageUpdateParameterivBRCM)
   {
      EGLint rect[4] = { (EGLint)x, (EGLint)y, (EGLint)width, (EGLint)height };
      m_eglImageUpdateParameterivBRCM(Application::Instance()->GetContext().GetDisplay(),
                                m_eglImage, EGL_IMAGE_UPDATE_CONTROL_CHANGED_REGION_BRCM, rect);
   }
#endif
}

void GLTexture::Lock()
{
#if EGL_BRCM_image_update_control
#ifdef EGL_IMAGE_UPDATE_CONTROL_SET_LOCK_STATE_BRCM
   if (m_eglImage != NULL && m_eglImageUpdateParameteriBRCM)
   {
      m_eglImageUpdateParameteriBRCM(Application::Instance()->GetContext().GetDisplay(),
                                     m_eglImage, EGL_IMAGE_UPDATE_CONTROL_SET_LOCK_STATE_BRCM,
                                     EGL_IMAGE_UPDATE_CONTROL_LOCK_BRCM);
   }
#endif
#endif
}

void GLTexture::Unlock()
{
#if EGL_BRCM_image_update_control
#ifdef EGL_IMAGE_UPDATE_CONTROL_SET_LOCK_STATE_BRCM
   if (m_eglImage != NULL && m_eglImageUpdateParameteriBRCM)
   {
      m_eglImageUpdateParameteriBRCM(Application::Instance()->GetContext().GetDisplay(),
                                     m_eglImage, EGL_IMAGE_UPDATE_CONTROL_SET_LOCK_STATE_BRCM,
                                     EGL_IMAGE_UPDATE_CONTROL_UNLOCK_BRCM);
   }
#endif
#endif
}

void GLSamplerState::Init()
{
#ifdef BSG_USE_ES3
   glGenSamplers(1, &m_sampler);
   glSamplerParameteri(m_sampler, GL_TEXTURE_WRAP_S,     m_wrapS    );
   glSamplerParameteri(m_sampler, GL_TEXTURE_WRAP_T,     m_wrapT    );
   glSamplerParameteri(m_sampler, GL_TEXTURE_MIN_FILTER, m_minFilter);
   glSamplerParameteri(m_sampler, GL_TEXTURE_MAG_FILTER, m_magFilter);
   // TODO could add support for these
   // glSamplerParameteri(m_sampler, GL_TEXTURE_MAX_LOD, m_maxLOD);
   // glSamplerParameteri(m_sampler, GL_TEXTURE_MIN_LOD, m_minLOD);
#endif
}

void GLSamplerState::Select(const GLProgram &prog, const std::string &name, TextureHandle t) const
{
   if (&prog != m_prog)
   {
      GLint loc = prog.GetUniformLocation(name);
      if (loc >= 0)
         glUniform1i(loc, m_unit);

      m_prog = &prog;
   }

   glActiveTexture(GL_TEXTURE0 + m_unit);
   t->Bind(m_target);

#ifdef BSG_USE_ES3
   glBindSampler(m_unit, m_sampler);
#else
   if (m_paramsDirty || t != m_texture)
   {
      glTexParameteri(m_target, GL_TEXTURE_WRAP_S,     m_wrapS    );
      glTexParameteri(m_target, GL_TEXTURE_WRAP_T,     m_wrapT    );
      glTexParameteri(m_target, GL_TEXTURE_MIN_FILTER, m_minFilter);
      glTexParameteri(m_target, GL_TEXTURE_MAG_FILTER, m_magFilter);

      m_paramsDirty = false;
   }
#endif

   m_texture = t;
}

}
