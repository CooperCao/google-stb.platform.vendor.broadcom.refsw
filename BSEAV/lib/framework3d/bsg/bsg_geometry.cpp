/******************************************************************************
 *   (c)2011-2012 Broadcom Corporation
 *
 * This program is the proprietary software of Broadcom Corporation and/or its
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
