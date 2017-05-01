/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef __HELLO_CUBE1_H__
#define __HELLO_CUBE1_H__

#include "bsg_application.h"
#include "bsg_animatable.h"
#include "bsg_animator.h"
#include "bsg_animation_list.h"

class HelloCubeApp1 : public bsg::Application
{
public:
   HelloCubeApp1(bsg::Platform &platform);

   // Overridden methods
   virtual void RenderFrame();
   virtual void KeyEventHandler(bsg::KeyEvents &queue);
   virtual bool UpdateFrame(int32_t *idleMs);
   virtual void ResizeHandler(uint32_t width, uint32_t height);

private:
   bsg::SceneNodeHandle m_root;
   bsg::CameraHandle    m_camera;
};

#endif /* __HELLO_CUBE1_H__ */
