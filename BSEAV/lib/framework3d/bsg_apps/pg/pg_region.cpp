/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "pg_region.h"

using namespace pg;
using namespace bsg;

bool Region::IntersectsInterval(const bsg::Time &start, const bsg::Time &finish) const
{
   bsg::Time finishTime = m_startTime + m_duration;

   if (start >= m_startTime && start <= finishTime)
      return true;

   if (finish >= m_startTime && finish <= finishTime)
      return true;

   if (start < m_startTime && finish > finishTime)
      return true;

   return false;
}

bool Region::ContainsTime(const bsg::Time &time) const
{
   bsg::Time finishTime = m_startTime + m_duration;

   if (time >= m_startTime && time <= finishTime)
      return true;

   return false;
}

Region Region::Enlarged(const bsg::Time &extendedTime, uint32_t channels) const
{
   Region r = *this;

   r.m_startTime = r.m_startTime - extendedTime;
   r.m_duration = r.m_duration + (extendedTime * 2.0f);
   r.m_startIndex = r.m_startIndex - (float)channels;
   r.m_numChannels = r.m_numChannels + (channels * 2);

   return r;
}

bool Region::operator==(const Region &rhs) const
{
   return m_startTime == rhs.m_startTime &&
          m_startIndex == rhs.m_startIndex &&
          m_duration == rhs.m_duration &&
          m_numChannels == rhs.m_numChannels;
}

bool Region::operator!=(const Region &rhs) const
{
   return !operator==(rhs);
}
