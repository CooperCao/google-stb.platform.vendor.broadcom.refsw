/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef __BSG_RENDER_OPTIONS_H__
#define __BSG_RENDER_OPTIONS_H__

#include "bsg_common.h"

namespace bsg
{

//! Settings that are passed to RenderSceneGraph() to control behaviour of the scene graph traversal.
//! Currently controls view frustum culling which is off by default.
//! View frustum culling eliminates objects whose bounding box doesn't intersect the camera's view volume.
//! View frustum culling requires extra calculations during graph traversal, so should only be used when there are
//! a lot of objects outside the field of view.  If an application can determine visibility, it is probably more efficient
//! for it to do so.
//! View frustum culling is disabled in quad mode.
class RenderOptions
{
public:
   RenderOptions() :
      m_enableViewFrustumCull(false)
   {}

   void SetEnableViewFrustumCull(bool b);

   bool GetEnableViewFrustumCull() const
   {
      return m_enableViewFrustumCull;
   }

private:
   bool  m_enableViewFrustumCull;
};

}

#endif /* __BSG_RENDER_OPTIONS_H__ */
