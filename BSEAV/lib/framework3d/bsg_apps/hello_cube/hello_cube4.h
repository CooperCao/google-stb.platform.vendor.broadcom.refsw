/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef __HELLO_CUBE4_H__
#define __HELLO_CUBE4_H__

#include "bsg_application.h"
#include "bsg_animatable.h"
#include "bsg_animator.h"
#include "bsg_animation_list.h"

class HelloCubeApp4 : public bsg::Application
{
public:
   HelloCubeApp4(bsg::Platform &platform);

private:
   // Overridden methods
   virtual void RenderFrame();
   virtual void KeyEventHandler(bsg::KeyEvents &queue);
   virtual bool UpdateFrame(int32_t *idleMs);
   virtual void ResizeHandler(uint32_t width, uint32_t height);

private:
   bsg::SceneNodeHandle m_root;
   bsg::CameraHandle    m_camera;

   bsg::AnimationList   m_animList;
};

#endif /* __HELLO_CUBE4_H__ */
