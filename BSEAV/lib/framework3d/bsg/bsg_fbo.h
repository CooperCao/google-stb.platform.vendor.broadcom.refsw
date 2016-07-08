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

#ifndef __BSG_FBO_H__
#define __BSG_FBO_H__

#include "bsg_common.h"
#include "bsg_gl.h"
#include "bsg_gl_texture.h"

#include "bsg_compiler_quirks.h"

namespace bsg
{

class Renderbuffer;
class Framebuffer;

// @cond
struct RenderbufferTraits
{
   typedef Renderbuffer         Base;
   typedef Renderbuffer         Derived;
   typedef RenderbufferTraits   BaseTraits;
};

struct FramebufferTraits
{
   typedef Framebuffer         Base;
   typedef Framebuffer         Derived;
   typedef FramebufferTraits   BaseTraits;
};
// @endcond

//! @addtogroup handles
//! @{
typedef Handle<RenderbufferTraits>  RenderbufferHandle;
typedef Handle<FramebufferTraits>   FramebufferHandle;
//! @}


//! Wraps up an OpenGL renderbuffer
class Renderbuffer : public NoCopy, public RefCount
{
   friend class Handle<RenderbufferTraits>;

public:
   //! Specify the storage requirements for this buffer
   void Storage(GLenum internalFormat, GLsizei width, GLsizei height)
   {
      Bind();
      glRenderbufferStorage(GL_RENDERBUFFER, internalFormat, width, height);
   }

   void Storage(GLenum internalFormat, GLsizei width, GLsizei height, GLsizei samples)
   {
      Bind();

      if (samples == 0)
         glRenderbufferStorage(GL_RENDERBUFFER, internalFormat, width, height);
      else
#ifdef GL_EXT_multisampled_render_to_texture
      {
         if (s_glRenderbufferStorageMultisampleEXT == NULL)
            s_glRenderbufferStorageMultisampleEXT = (PFNGLRENDERBUFFERSTORAGEMULTISAMPLEEXTPROC)eglGetProcAddress("glRenderbufferStorageMultisampleEXT");

         if (s_glRenderbufferStorageMultisampleEXT != NULL)
            s_glRenderbufferStorageMultisampleEXT(GL_RENDERBUFFER, samples, internalFormat, width, height);
         else
            BSG_THROW("Function glRenderbufferStorageMultisampleEXT not found");
      }
#else
      BSG_THROW("GL_EXT_multisampled_render_to_texture extention not supported");
#endif
   }

   //! Delete the buffer
   ~Renderbuffer()
   {
      glDeleteRenderbuffers(1, &m_id);
   }

   //! Select this buffer
   void Bind() const
   {
      glBindRenderbuffer(GL_RENDERBUFFER, m_id);
   }

   //! Attach this buffer to the bound framebuffer
   void FramebufferRenderbuffer(GLenum attachment) const
   {
      glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachment, GL_RENDERBUFFER, m_id);
   }

protected:
   //! Generate a render buffer
   Renderbuffer()
   {
      glGenRenderbuffers(1, &m_id);
   }

private:
   GLuint   m_id;
#ifdef GL_EXT_multisampled_render_to_texture
   static PFNGLRENDERBUFFERSTORAGEMULTISAMPLEEXTPROC s_glRenderbufferStorageMultisampleEXT;
#endif

};

// @cond
class AttachmentImpl
{
public:
   AttachmentImpl(GLenum attachment) :
      m_attachment(attachment)
   {}

   virtual ~AttachmentImpl() {}

   virtual void Attach(GLsizei samples = 0) const = 0;

   GLenum GetAttachment() const { return m_attachment; }

private:
   GLenum   m_attachment;
};

//! Renderbuffer attachment
class RenderbufferAttachment : public AttachmentImpl
{
public:
   RenderbufferAttachment(RenderbufferHandle buffer, GLenum attachment) :
      AttachmentImpl(attachment),
      m_buffer(buffer)
   {}

   virtual void Attach(GLsizei samples = 0) const
   {
      samples = 0; // Unused: set to remove warning
      m_buffer->FramebufferRenderbuffer(GetAttachment());
   }

private:
   RenderbufferHandle   m_buffer;
};

//! Texture attachment
class TextureAttachment : public AttachmentImpl
{
public:
   TextureAttachment(TextureHandle texture, GLenum target, GLuint level, GLenum attachment) :
      AttachmentImpl(attachment),
      m_texture(texture),
      m_target(target),
      m_level(level)
   {}

   virtual void Attach(GLsizei samples = 0) const
   {
      if (samples == 0)
         m_texture->FramebufferTexture2D(GetAttachment(), m_target, m_level);
      else
#ifdef GL_EXT_multisampled_render_to_texture
      {
         if (eglGetProcAddress("glFramebufferTexture2DMultisampleEXT") != 0)
            m_texture->FramebufferTexture2DMultisample(GetAttachment(), m_target, m_level, samples);
         else
            BSG_THROW("Function glFramebufferTexture2DMultisampleEXT not found");
      }
#else
      BSG_THROW("GL_EXT_multisampled_render_to_texture extention not supported");
#endif
   }

private:
   TextureHandle  m_texture;
   GLenum         m_target;
   GLuint         m_level;
};

// @endcond

//! Attachment abstracts the notions of texture and renderbuffer attachments
class Attachment : public NoCopy
{
public:
   //! Make a 2D texture attachment
   Attachment(TextureHandle texture, GLenum attachment, GLuint level = 0)
   {
      m_impl = new TextureAttachment(texture, GL_TEXTURE_2D, level, attachment);
   }

   //! Make a 2D or cube-map texture attachment
   Attachment(TextureHandle texture, GLenum attachment, GLenum texTarget, GLuint level = 0)
   {
      m_impl = new TextureAttachment(texture, texTarget, level, attachment);
   }

   //! Make a renderbuffer attachment
   Attachment(const RenderbufferHandle &buffer, GLenum attachment)
   {
      m_impl = new RenderbufferAttachment(buffer, attachment);
   }

   //! Used by framebuffers to attach the attachments
   void Attach(GLsizei samples = 0) const
   {
      m_impl->Attach(samples);
   }

   //! Delete
   ~Attachment() { delete m_impl; }

private:
   AttachmentImpl *m_impl;
};

extern GLuint GetRenderTarget();

// @cond
class BindFBO
{
public:
   BindFBO(GLuint id) :
      m_old(GetRenderTarget()),
      m_id(id)
   {
      if (m_old != m_id)
         glBindFramebuffer(GL_FRAMEBUFFER, m_id);
   }

   ~BindFBO()
   {
      if (m_id != m_old)
         glBindFramebuffer(GL_FRAMEBUFFER, m_old);
   }

private:
   GLuint   m_old;
   GLuint   m_id;
};

// @endcond

//! Framebuffers are the targets for rendering.  The Framebuffer class is a wrapper over the GL framebuffer object.
//! When a Framebuffer is created, a GL frambuffer object is generated, and when deleted the GL framebuffer object is
//! also deleted.
//!
//! A FramebufferHandle can be passed to the Application method SetRenderTarget().  Thenceforce, all rendering
//! will go to the framebuffer rather than to any previous framebuffer or the default (on-screen) target.
class Framebuffer : public NoCopy, public RefCount
{
   friend class Handle<FramebufferTraits>;
   friend class Application;                 // Give access to Bind() and BindDefault()

public:
   ~Framebuffer()
   {
      glDeleteFramebuffers(1, &m_id);
   }

   //! Attach a single item.
   void Attach(const Attachment &attachment) const
   {
      BindFBO  bind(m_id);

      attachment.Attach(m_samples);
   }

   //! Attach two attachments
   void Attach(const Attachment &attachment1, const Attachment &attachment2)
   {
      BindFBO  bind(m_id);

      attachment1.Attach(m_samples);
      attachment2.Attach(m_samples);
   }

   //! Attach three attachments
   void Attach(const Attachment &attachment1, const Attachment &attachment2, const Attachment &attachment3)
   {
      BindFBO  bind(m_id);

      attachment1.Attach(m_samples);
      attachment2.Attach(m_samples);
      attachment3.Attach(m_samples);
   }

   //! Detach from specified attachment point
   void Detach(GLenum attachment) const
   {
      BindFBO  bind(m_id);

      glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachment, GL_RENDERBUFFER, 0);
   }

   //! Detach from up to three attachment points
   void Detach(GLenum attachment1, GLenum attachment2, GLenum attachment3 = GL_ZERO) const
   {
      BindFBO  bind(m_id);

      glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachment1, GL_RENDERBUFFER, 0);
      glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachment2, GL_RENDERBUFFER, 0);

      if (attachment3 != GL_ZERO)
         glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachment3, GL_RENDERBUFFER, 0);
   }

   //! Check the framebuffer for completeness -- returns GL_FRAMEBUFFER_COMPLETE for success.
   GLenum Check() const
   {
      BindFBO  bind(m_id);

      return glCheckFramebufferStatus(GL_FRAMEBUFFER);
   }

   //! Return true of the DiscardFramebuffer extension is supported
   bool DiscardSupported() const
   {
#ifndef EMULATED
#ifdef BSG_USE_ES3
      return true;
#else
      if (s_discardFrameBuffer == NULL)
         s_discardFrameBuffer = (PFNGLDISCARDFRAMEBUFFEREXTPROC)eglGetProcAddress("glDiscardFramebufferEXT");

      return s_discardFrameBuffer != NULL;
#endif
#else
      return false;
#endif
   }

   //! Mark attachment points as being discardable.  They are not actually discarded by this method.
   //! Any previously marked attachments are unmarked.
   void Discardable(GLenum attachment1, GLenum attachment2 = GL_ZERO, GLenum attachment3 = GL_ZERO)
   {
      m_discardableBuffers.clear();
      m_discardableBuffers.push_back(attachment1);

      if (attachment2 != GL_ZERO)
         m_discardableBuffers.push_back(attachment2);

      if (attachment3 != GL_ZERO)
         m_discardableBuffers.push_back(attachment3);
   }

   //! Discard the buffers marked as discardable.
   void Discard() const
   {
      BindFBO  bind(m_id);

#ifndef EMULATED
#ifdef BSG_USE_ES3
      glInvalidateFramebuffer(GL_FRAMEBUFFER, m_discardableBuffers.size(), &m_discardableBuffers[0]);
#else
      if (DiscardSupported())
      {
         if (m_discardableBuffers.size() > 0)
            s_discardFrameBuffer(GL_FRAMEBUFFER, m_discardableBuffers.size(), &m_discardableBuffers[0]);
      }
#endif
#endif
   }

   GLuint GetId() const
   {
      return m_id;
   }

   void SetSampleNumber(GLsizei samples)
   {
      m_samples = samples;
   }

protected:
   Framebuffer():
      m_samples(0)
   {
      glGenFramebuffers(1, &m_id);
   }

   // These two methods used by the application when a render target is set
   void Bind() const
   {
      glBindFramebuffer(GL_FRAMEBUFFER, m_id);
   }

   static void BindDefault()
   {
      glBindFramebuffer(GL_FRAMEBUFFER, 0);
   }

private:
#ifndef EMULATED
#ifndef BSG_USE_ES3
   static PFNGLDISCARDFRAMEBUFFEREXTPROC s_discardFrameBuffer;
#endif
#endif
   GLuint               m_id;
   std::vector<GLenum>  m_discardableBuffers;
   GLsizei              m_samples;
};

// @endcond
}

#endif /* __BSG_FBO_H__ */

