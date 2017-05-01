/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef __BSG_GEOMETRY_H__
#define __BSG_GEOMETRY_H__

#include <stdint.h>
#include <vector>
#include <memory>
#include "bsg_common.h"
#include "bsg_box.h"
#include "bsg_library.h"
#include "bsg_surface.h"
#include "bsg_pass_state.h"
#include "bsg_lod_range.h"

namespace bsg
{

class Visitor;
class Geometry;

// @cond
struct GeometryTraits
{
   typedef Geometry        Base;
   typedef Geometry        Derived;
   typedef GeometryTraits  BaseTraits;
};
// @endcond

//! @addtogroup handles
//! @{
typedef Handle<GeometryTraits>   GeometryHandle;
//! @}

typedef std::vector<GeometryHandle> Geometries;

/** @addtogroup scenegraph
@{
*/

class DrawCallback
{
public:
   virtual ~DrawCallback() {}
   virtual bool OnDraw() = 0;
};

// @cond
//! An object that composes a surface handle and a material handle.
//! Note that ownership of the callback is passed to when the object is assigned/copy constructed
class SurfMatBinding
{
public:
   SurfMatBinding(SurfaceHandle surf, MaterialHandle mat, DrawCallback *callback) :
      m_surface(surf),
      m_material(mat),
      m_callback(callback),
      m_sortPriority(0)
   {}

   SurfMatBinding(SurfaceHandle surf, MaterialHandle mat, int32_t sortPriority, DrawCallback *callback) :
      m_surface(surf),
      m_material(mat),
      m_callback(callback),
      m_sortPriority(sortPriority)
   {}

   ~SurfMatBinding()
   {
      // Don't delete the callback here, it is owned by the geometry
   }

   SurfaceHandle     m_surface;
   MaterialHandle    m_material;
   DrawCallback      *m_callback;
   int32_t           m_sortPriority;
};
// @endcond

//! A Geometry can be thought of as a physical object.  It consists of a number of surfaces (the shape of the object) with their
//! materials (the visual properties of the object).
//! A Geometry consists of a list of Surface/Material pairs.  It also manages the LOD mechanism by associating a range
//! of screen sizes at whhich this item should be drawn.
class Geometry : public RefCount
{
   friend class Handle<GeometryTraits>;

public:
   virtual ~Geometry();

   //! Sets the "reflected" flag for this geometry which controls how the
   //! winding orders of surfaces are treated
   void SetReflected(bool reflected)
   {
      m_reflected = reflected;
   }

   //! Gets the "reflected" flag for this geometry which controls how the
   //! winding orders of surfaces are treated
   bool GetReflected() const
   {
      return m_reflected;
   }

   //! Sets the back-face culling modes for all the contained surfaces.
   void SetCull(const CullMode &cull)
   {
      for (uint32_t i = 0; i < NumSurfaces(); ++i)
         GetSurface(i)->SetCull(cull);
   }

   //! Sets the sort priority for all the contained surfaces.
   void SetSortPriority(int32_t priority)
   {
      for (uint32_t i = 0; i < NumSurfaces(); ++i)
         m_surfaces[i].m_sortPriority = priority;
   }

   // @cond
   const LODRange &GetLodRange() const { return m_lodRange; }
   LODRange &GetLodRange()             { return m_lodRange; }
   // @endcond

   //! @name Accessors
   //! @{
   void           AppendSurface(SurfaceHandle surf, MaterialHandle material, DrawCallback *newCallback = 0);
   void           AppendSurfaceWithSortPriority(SurfaceHandle surf, MaterialHandle material, int32_t priority,
                                                DrawCallback *newCallback = 0);
   void           Clear();
   uint32_t       NumSurfaces() const;
   SurfaceHandle  GetSurface(uint32_t n) const;
   void           SetSurface(uint32_t n, SurfaceHandle surf, MaterialHandle material, DrawCallback *newCallback = 0);
   void           SetSurface(uint32_t n, SurfaceHandle surf);
   MaterialHandle GetMaterial(uint32_t n) const;
   void           SetMaterial(uint32_t n, MaterialHandle material);
   DrawCallback   *GetDrawCallback(uint32_t n) const;
   void           SetDrawCallback(uint32_t n, DrawCallback *cb);
   int32_t        GetSortPriority(uint32_t n) const;
   //! @}

#ifdef BSG_USE_ES3
   void           SetNumInstances(uint32_t n);
#endif

protected:
   Geometry() :
      m_reflected(false)
   {}

private:
   std::vector<SurfMatBinding>   m_surfaces;

   // Marks geometry as a reflection and flips culling decisions
   bool                          m_reflected;

   // LOD management
   LODRange                      m_lodRange;
};

/** @} */
}

#endif
