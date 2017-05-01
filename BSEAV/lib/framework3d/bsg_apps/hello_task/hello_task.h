/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef __HELLO_TASK_H__
#define __HELLO_TASK_H__

#include "bsg_application.h"
#include <memory>

class HelloTaskApp : public bsg::Application
{
public:
   HelloTaskApp(bsg::Platform &platform);

   enum
   {
      COUNT = 1000
   };

   // Used during callbacks
   void SetBoxSize(uint32_t size) const;
   void Done() const;

   // Overridden methods
   virtual void RenderFrame();
   virtual void KeyEventHandler(bsg::KeyEvents &queue);
   virtual void ResizeHandler(uint32_t width, uint32_t height);
   virtual bool UpdateFrame(int32_t *idleMs);

private:
   bsg::SceneNodeHandle       m_root;
   bsg::CameraHandle          m_camera;
   bsg::MaterialHandle        m_material;

   std::auto_ptr<bsg::Tasker> m_tasker;
};

#endif /* __HELLO_TASK_H__ */
