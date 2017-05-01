/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "bsg_fbo.h"
#include "bsg_application.h"

#ifndef EMULATED
#ifndef BSG_USE_ES3
   PFNGLDISCARDFRAMEBUFFEREXTPROC bsg::Framebuffer::s_discardFrameBuffer = NULL;
#endif
#endif

#ifdef GL_EXT_multisampled_render_to_texture
   PFNGLRENDERBUFFERSTORAGEMULTISAMPLEEXTPROC bsg::Renderbuffer::s_glRenderbufferStorageMultisampleEXT = NULL;
#endif

namespace bsg
{

GLuint GetRenderTarget()
{
   const FramebufferHandle *fbo = Application::Instance()->GetRenderTarget();

   return fbo == 0 ? 0 : (*fbo)->GetId();
}

}
