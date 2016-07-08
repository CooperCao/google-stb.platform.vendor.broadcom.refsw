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
