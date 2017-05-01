/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "bsg_lod_range.h"
#include "bsg_application.h"

#include <math.h>

using namespace bsg;

#include "bsg_application.h"

bool LODRange::InRange(const SemanticData &semData, const Bound &bound) const
{
   if (!m_set)
      return true;

   const Application &app = *Application::Instance();

   float r = bound.GetRadius();
   Vec3  c = bound.GetCenter();

   Vec3  bl = c - Vec3(r, r, 0.0f);
   Vec3  tr = c + Vec3(r, r, 0.0f);

   Vec2  bl1 = (semData.GetProjMatrix() * bl.Lift(1.0f)).Proj().Drop();
   Vec2  tr1 = (semData.GetProjMatrix() * tr.Lift(1.0f)).Proj().Drop();

   // The projected size tends to be an overestimate due to way bounding volumes grow
   // because of being conservative when transformed/calculated.
   // Thge factor of 3 attempts to address this issue for typical use cases.
   float ndcSize = Length((bl1 - tr1) * (float)app.GetOptions().GetHeight() / 3.0f);

   return ndcSize >= m_minSize && ndcSize < m_maxSize;
}
