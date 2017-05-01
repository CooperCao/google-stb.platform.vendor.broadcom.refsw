/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef __HELLO_TEXT_H__
#define __HELLO_TEXT_H__

#include "bsg_application.h"
#include "bsg_animation_list.h"

class HelloTextApp : public bsg::Application
{
public:
   HelloTextApp(bsg::Platform &platform);

private:
   // Overridden methods
   virtual void RenderFrame();
   virtual void KeyEventHandler(bsg::KeyEvents &queue);
   virtual void ResizeHandler(uint32_t width, uint32_t height);
   virtual bool UpdateFrame(int32_t *idleMs);

private:
   bsg::SceneNodeHandle m_root;
   bsg::SceneNodeHandle m_cameraNode;
   bsg::CameraHandle    m_camera;

   bsg::FontHandle      m_font2D;
   bsg::AnimatableVec2  m_textPos;

   bsg::AnimationList   m_animList;
};

#endif /* __HELLO_TEXT_H__ */
