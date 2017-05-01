/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "bsg_geometry.h"
#include "bsg_visitor.h"
#include "bsg_surface.h"
#include "bsg_exception.h"

namespace bsg
{

Geometry::~Geometry()
{
   Clear();
}

void Geometry::AppendSurface(SurfaceHandle surface, MaterialHandle material, DrawCallback *newCallback)
{
   m_surfaces.push_back(SurfMatBinding(surface, material, newCallback));
}

void Geometry::AppendSurfaceWithSortPriority(SurfaceHandle surface, MaterialHandle material, int32_t priority,
                                             DrawCallback *newCallback /*= 0*/)
{
   m_surfaces.push_back(SurfMatBinding(surface, material, priority, newCallback));
}

void Geometry::Clear()
{
   for (uint32_t i = 0; i < m_surfaces.size(); ++i)
      delete m_surfaces[i].m_callback;

   m_surfaces.clear();
}

uint32_t Geometry::NumSurfaces() const
{
   return m_surfaces.size();
}

SurfaceHandle Geometry::GetSurface(uint32_t n) const
{
#ifndef NDEBUG
   if (n >= NumSurfaces())
      BSG_THROW("Invalid index");
#endif

   return m_surfaces[n].m_surface;
}

void Geometry::SetSurface(uint32_t n, SurfaceHandle surface, MaterialHandle material, DrawCallback *newCallback)
{
#ifndef NDEBUG
   if (n >= NumSurfaces())
      BSG_THROW("Invalid index");
#endif

   delete m_surfaces[n].m_callback;

   m_surfaces[n] = SurfMatBinding(surface, material, newCallback);
}

void Geometry::SetSurface(uint32_t n, SurfaceHandle surface)
{
#ifndef NDEBUG
   if (n >= NumSurfaces())
      BSG_THROW("Invalid index");
#endif

   delete m_surfaces[n].m_callback;

   m_surfaces[n].m_surface = surface;
}

MaterialHandle Geometry::GetMaterial(uint32_t n) const
{
#ifndef NDEBUG
   if (n >= NumSurfaces())
      BSG_THROW("Invalid index");
#endif

   return m_surfaces[n].m_material;
}

void Geometry::SetMaterial(uint32_t n, MaterialHandle material)
{
#ifndef NDEBUG
   if (n >= NumSurfaces())
      BSG_THROW("Invalid index");
#endif

   m_surfaces[n].m_material = material;
}

DrawCallback *Geometry::GetDrawCallback(uint32_t n) const
{
#ifndef NDEBUG
   if (n >= NumSurfaces())
      BSG_THROW("Invalid index");
#endif

   return m_surfaces[n].m_callback;
}

void Geometry::SetDrawCallback(uint32_t n, DrawCallback *cb)
{
#ifndef NDEBUG
   if (n >= NumSurfaces())
      BSG_THROW("Invalid index");
#endif

   delete m_surfaces[n].m_callback;
   m_surfaces[n].m_callback = cb;
}

int32_t Geometry::GetSortPriority(uint32_t n) const
{
#ifndef NDEBUG
   if (n >= NumSurfaces())
      BSG_THROW("Invalid index");
#endif

   return m_surfaces[n].m_sortPriority;
}

#ifdef BSG_USE_ES3
void Geometry::SetNumInstances(uint32_t n)
{
   for (uint32_t i = 0; i < m_surfaces.size(); ++i)
      m_surfaces[i].m_surface->SetNumInstances(n);
}
#endif


}
