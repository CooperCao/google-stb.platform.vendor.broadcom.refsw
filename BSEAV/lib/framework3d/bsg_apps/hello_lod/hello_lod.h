/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef __HELLO_LOD_H__
#define __HELLO_LOD_H__

#include "bsg_application.h"
#include "bsg_animation_list.h"

class CustomArgumentParser;

class HelloLod : public bsg::Application
{
public:
   HelloLod(bsg::Platform &platform, const CustomArgumentParser &options);

   // Overridden methods
   virtual void RenderFrame();
   virtual void KeyEventHandler(bsg::KeyEvents &queue);
   virtual void ResizeHandler(uint32_t width, uint32_t height);
   virtual bool UpdateFrame(int32_t *idleMs);

private:
   void        ToggleLOD();

private:
   bsg::SceneNodeHandle      m_root;
   bsg::SceneNodeHandle      m_obj;
   bsg::SceneNodeHandle      m_objLOD;
   bsg::SceneNodeHandle      m_objNoLOD;
   bsg::FontHandle           m_font;
   bsg::CameraHandle         m_camera;

   bool                      m_showChanges;
   bool                      m_useLOD;

   bsg::AnimationList        m_animList;
};

#endif /* __HELLO_LOD_H__ */
