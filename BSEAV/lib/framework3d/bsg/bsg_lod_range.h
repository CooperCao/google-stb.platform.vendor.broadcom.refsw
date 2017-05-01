/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef __BSG_LOD_RANGE_H__
#define __BSG_LOD_RANGE_H__

#include "bsg_common.h"

namespace bsg
{

class SemanticData;
class Bound;

// @cond

class LODRange
{
public:
   LODRange() :
      m_minSize(0.0f),
      m_maxSize(0.0f),
      m_set(false)
   {}

   void SetRange(float screenMinSize, float screenMaxSize)
   {
      m_minSize = screenMinSize;
      m_maxSize = screenMaxSize;
      m_set   = true;
   }

   bool InRange(const SemanticData &semData, const Bound &bound) const;

   bool IsSet() const
   {
      return m_set;
   }

private:
   float m_minSize;
   float m_maxSize;
   bool  m_set;
};

// @endcond

}

#endif /* __BSG_LOD_RANGE_H__ */
