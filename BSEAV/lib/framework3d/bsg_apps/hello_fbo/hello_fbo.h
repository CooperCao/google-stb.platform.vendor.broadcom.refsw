/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef __HELLO_FBO_H__
#define __HELLO_FBO_H__

#include "bsg_application.h"
#include "bsg_animation_list.h"
#include "bsg_fbo.h"

class HelloFBO : public bsg::Application
{
public:
   HelloFBO(bsg::Platform &platform);

private:
   // Overridden methods
   virtual void RenderFrame();
   virtual void KeyEventHandler(bsg::KeyEvents &queue);
   virtual void ResizeHandler(uint32_t width, uint32_t height);
   virtual bool UpdateFrame(int32_t *idleMs);

   enum
   {
      FBO_WIDTH  = 512,
      FBO_HEIGHT = 512
   };

private:
   bsg::SceneNodeHandle      m_root;
   bsg::SceneNodeHandle      m_cameraNode;
   bsg::SceneNodeHandle      m_cubeNode;
   bsg::CameraHandle         m_camera;
   bsg::MaterialHandle       m_cubeMat;

   bsg::TextureHandle        m_textureFBO;
   bsg::TextureHandle        m_textureLogo;
   bsg::FramebufferHandle    m_fbo;
   bsg::RenderbufferHandle   m_depth;
   bool                      m_discard;

   bsg::AnimationList        m_animList;
};

#endif /* __HELLO_FBO_H__ */
