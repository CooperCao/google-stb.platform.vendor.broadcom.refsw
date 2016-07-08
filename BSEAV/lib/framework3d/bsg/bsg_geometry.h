/******************************************************************************
 *   Broadcom Proprietary and Confidential. (c)2011-2012 Broadcom.  All rights reserved.
 *
 * This program is the proprietary software of Broadcom and/or its
 * licensors, and may only be used, duplicated, modified or distributed
 * pursuant to the terms and conditions of a separate, written license
 * agreement executed between you and Broadcom (an "Authorized License").
 * Except as set forth in an Authorized License, Broadcom grants no license
 * (express or implied), right to use, or waiver of any kind with respect to
 * the Software, and Broadcom expressly reserves all rights in and to the
 * Software and all intellectual property rights therein.  IF YOU HAVE NO
 * AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY,
 * AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE
 * SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use all
 * reasonable efforts to protect the confidentiality thereof, and to use this
 * information only in connection with your use of Broadcom integrated circuit
 * products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 * OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 * RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 * IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 * A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 * ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE
 * ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
 * ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 * INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 * RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 * HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 * EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 * WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 * FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *****************************************************************************/
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

