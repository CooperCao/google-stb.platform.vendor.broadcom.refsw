/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef __BSG_LOD_GROUP_H__
#define __BSG_LOD_GROUP_H__

#include "bsg_common.h"
#include "bsg_geometry.h"

#undef min
#undef max

#include <limits>

namespace bsg
{

//! Lots of geometric detail is only useful if the object is drawn at a reasonable size on the display.
//! The idea behind geometric level of detail (LOD) is to select different models according to the
//! projected size of the object.
//! The implementation uses an approximate method based on the size of the bounding volume.  This usually
//! results in an overestimate of the objects screen size, but this is preferable to an underestimate
//! which would lead to chunky looking models and visible transitions between LOD levels.
//!
//! Applications can select the level of detail to be used when rendering an object via:
//! \code
//! LODGroup  lodGroup(GetOptions().GetHeight());
//!
//! lodGroup.Begin();
//! lodGroup.Add(geom1, 200.0f); // 200 and above
//! lodGroup.Add(geom2, 50.0f);  // 50 to 200
//! lodGroup.Add(geom3, 12.0f);  // 12 to 50
//! lodGroup.Add(geom4);         // 0 to 12
//! lodGroup.End();
//! \endcode
//!
//! This selects geom1 for projected sizes of 200 pixels, geom2 for 50 to 200 and so on.
class LODGroup
{
public:
   LODGroup() :
      m_inBegin(false),
      m_previous(std::numeric_limits<float>::max())
   {
   }

   ~LODGroup()
   {
      if (m_inBegin)
         BSG_THROW("LODGroup destroyed whilst still in a begin/end");
   }

   void Begin()
   {
      m_inBegin = true;
   }

   void End();

   void Add(GeometryHandle geom, float screenMinSize)
   {
      if (!m_inBegin)
         BSG_THROW("LODGroup Add called outside of a begin/end");

      if (screenMinSize >= m_previous)
         BSG_THROW("LODGroup Add called in wrong order.  Highest values must be set first");

      m_geometries.push_back(geom);
      geom->GetLodRange().SetRange(screenMinSize, m_previous);
      m_previous = screenMinSize;
   }

   void Add(GeometryHandle geom)
   {
      Add(geom, 0.0f);
   }

private:
   Geometries  m_geometries;
   bool        m_inBegin;
   float       m_previous;
};

}

#endif /* __BSG_LOD_GROUP_H__ */
