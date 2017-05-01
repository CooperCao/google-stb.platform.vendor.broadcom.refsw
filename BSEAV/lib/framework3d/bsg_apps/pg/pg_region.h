/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef __PG_REGION_H__
#define __PG_REGION_H__

#include <stdint.h>
#include <iostream>

#include "bsg_time.h"
#include "bsg_animatable.h"

namespace pg
{

class Region
{
public:
   Region() :
      m_startIndex(0.0f),
      m_numChannels(0)
   {}

   Region(const bsg::Time &startTime, const bsg::Time &duration, uint32_t startIndex, uint32_t numChannels) :
      m_startTime(startTime),
      m_duration(duration),
      m_startIndex((float)startIndex),
      m_numChannels(numChannels)
   {}

   bool ContainsChannel(int32_t index, float fuzz = 0.0f) const
   {
      return index >= (m_startIndex - fuzz) && index < (m_startIndex + m_numChannels + fuzz);
   }

   bool Includes(const bsg::Time &t) const
   {
      return t >= m_startTime && t < (m_startTime + m_duration);
   }

   bool IntersectsInterval(const bsg::Time &start, const bsg::Time &finish) const;
   bool ContainsTime(const bsg::Time &time) const;

   Region Enlarged(const bsg::Time &extendedTime, uint32_t channels) const;

   void UpdateStartTime(const bsg::Time &time)    { m_startTime = time; }
   void UpdateStartChannel(int32_t channelIndex) { m_startIndex = (float)channelIndex; }

   const bsg::AnimatableTime &GetStartTime() const { return m_startTime; }
   bsg::AnimatableTime       &GetStartTime()       { return m_startTime; }

   bsg::Time GetFinishTime() const { return m_startTime + m_duration; }
   bsg::Time GetDuration() const   { return m_duration; }

   const bsg::AnimatableFloat &GetStartIndex() const { return m_startIndex; }
   bsg::AnimatableFloat       &GetStartIndex()       { return m_startIndex; }
   uint32_t                   GetNumChannels() const { return m_numChannels; }

   bool operator==(const Region &rhs) const;
   bool operator!=(const Region &rhs) const;

   friend std::ostream &operator<<(std::ostream &os, const Region &reg) { return os << "Region: " << reg.m_startTime.FloatSeconds() << ", " << reg.m_duration.FloatSeconds() << ", " << reg.m_startIndex << ", " << reg.m_numChannels << "\n"; }

private:
   bsg::AnimatableTime   m_startTime;      // Time window
   bsg::AnimatableTime   m_duration;
   bsg::AnimatableFloat  m_startIndex;     // Channel index window (float to allow scrolling)
   uint32_t              m_numChannels;
};

}

#endif
