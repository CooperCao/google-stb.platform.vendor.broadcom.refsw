/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef __GEOM_VIEWER_H__
#define __GEOM_VIEWER_H__

#include "bsg_application.h"
#include "bsg_animatable.h"
#include "bsg_animator.h"
#include "bsg_font.h"
#include "bsg_text.h"

#include "menu_carousel.h"

namespace bsg
{
   class SceneNode;
   class Camera;
}

class GeometryItem
{
public:
   GeometryItem(bsg::GeometryHandle geom, const std::string &info) :
      m_geometry(geom),
      m_infoString(info)
   {}

   const bsg::GeometryHandle &GetGeometry() const { return m_geometry;    }
   const std::string         &GetInfo()     const { return m_infoString;  }

private:
   bsg::GeometryHandle m_geometry;
   std::string         m_infoString;
};

class GeomViewer : public bsg::Application
{
public:
   GeomViewer(bsg::Platform &platform);

   // Overridden methods
   virtual bool UpdateFrame(int32_t *idleMs);
   virtual void RenderFrame();
   virtual void KeyEventHandler(bsg::KeyEvents &queue);
   virtual void MouseEventHandler(bsg::MouseEvents &queue);
   virtual void ResizeHandler(uint32_t width, uint32_t height);

private:
   bsg::SceneNodeHandle m_root;
   bsg::SceneNodeHandle m_camNode;
   bsg::CameraHandle    m_camera;
   bsg::FontHandle      m_font;

   bsg::AnimationList        m_animList;
   bsg::Auto<MenuCarousel>   m_carousel;

   std::vector<bsg::SceneNodeHandle>  m_menuItems;
   std::vector<GeometryItem>          m_geoms;
};

#endif /* __GEOM_VIEWER_H__ */
