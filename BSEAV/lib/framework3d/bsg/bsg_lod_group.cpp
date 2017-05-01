/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "bsg_lod_group.h"

namespace bsg
{

void LODGroup::End()
{
   if (!m_inBegin)
      BSG_THROW("LODGroup end called without corresponding begin");

   m_inBegin = false;

   Bound biggest(0.0f, Vec3());

   // Find the biggest bound and use that for the group.
   for (uint32_t g = 0; g < m_geometries.size(); ++g)
   {
      GeometryHandle geom  = m_geometries[g];
      uint32_t numSurfaces = geom->NumSurfaces();

      for (uint32_t s = 0; s < numSurfaces; ++s)
      {
         SurfaceHandle  surf = geom->GetSurface(s);

         if (surf->GetBound().GetRadius() > biggest.GetRadius())
            biggest = surf->GetBound();
      }
   }

   // Set all surfaces to use this bound
   for (uint32_t g = 0; g < m_geometries.size(); ++g)
   {
      GeometryHandle geom  = m_geometries[g];
      uint32_t numSurfaces = geom->NumSurfaces();

      for (uint32_t s = 0; s < numSurfaces; ++s)
      {
         SurfaceHandle  surf = geom->GetSurface(s);

         surf->SetBound(biggest);
      }
   }
}

}
