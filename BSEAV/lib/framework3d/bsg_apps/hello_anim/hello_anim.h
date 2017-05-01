/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef __HELLO_ANIM_H__
#define __HELLO_ANIM_H__

#include "bsg_application.h"
#include "bsg_animation_list.h"

class HelloAnimApp : public bsg::Application
{
public:
   HelloAnimApp(bsg::Platform &platform);

private:
   // Overridden methods
   virtual void RenderFrame();
   virtual void KeyEventHandler(bsg::KeyEvents &queue);
   virtual void ResizeHandler(uint32_t width, uint32_t height);
   virtual bool UpdateFrame(int32_t *idleMs);

private:
   bsg::SceneNodeHandle m_root;
   bsg::CameraHandle    m_camera;

   bsg::AnimationList   m_animList;
};

#endif /* __HELLO_ANIM_H__ */
