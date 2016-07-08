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

#include "pg.h"
#include "pg_channel_node.h"
#include "pg_info.h"
#include "pg_region.h"

#include <set>

#undef min

using namespace bsg;
using namespace pg;

ChannelNode::ChannelNode() :
   m_root(New),
   m_line(0xdeadbeef)
{
}

ChannelNode::ChannelNode(uint32_t line, const ChannelInfo &info) :
   m_root(New),
   m_name(info.GetName()),
   m_line(line),
   m_info(&info)
{
}

void ChannelNode::VisibilityCull(const Region &region)
{
   std::list<IntervalNode>::iterator iter;
   for (iter = m_interval.begin(); iter != m_interval.end(); ++iter)
   {
      IntervalNode   &interval = *iter;

      bool  visible = region.IntersectsInterval(interval.GetIntervalStartTime(), interval.GetLastProgFinishTime());

      interval.GetRoot()->SetVisible(visible);

      if (visible)
         interval.VisibilityCull(region);
   }
}

// Sorting functor for programs
struct SortByStartTime
{
   bool operator()(const ProgramInfo *p1, const ProgramInfo *p2)
   {
      return p1->GetStartTime() < p2->GetStartTime();
   };

};

static void GatherPrograms(std::list<const ProgramInfo *> *keepProgramList, const ChannelInfo *info, const Region &region)
{
   for (uint32_t p = 0; p < info->GetNumPrograms(); ++p)
   {
      const ProgramInfo &prog = info->GetProgram(p);

      if (region.IntersectsInterval(prog.GetStartTime(), prog.GetEndTime()))
      {
         // Crosses the keep region
         keepProgramList->push_back(&prog);
      }
   }

   keepProgramList->sort(SortByStartTime());
}

static void RationalisePrograms(std::vector<ProgramInfo> *keepPrograms, const std::list<const ProgramInfo *> &keepProgramList, const Region &region)
{
   // There is no information for this channel
   if (keepProgramList.size() == 0)
   {
      ProgramInfo      noSchedule(region.GetStartTime(), region.GetFinishTime(), Metrics::GetNoScheduleText(), Metrics::GetNoScheduleDescription());

      keepPrograms->push_back(noSchedule);
   }
   else
   {
      std::list<const ProgramInfo *>::const_iterator  current     = keepProgramList.begin();
      const Time                                      &firstStart = (*current)->GetStartTime();

      // Missing data at the start
      if (firstStart > region.GetStartTime())
      {
         ProgramInfo noInformation(region.GetStartTime(), firstStart, Metrics::GetNoProgrammeText(), Metrics::GetNoProgrammeDescription());

         keepPrograms->push_back(noInformation);
      }

      std::list<const ProgramInfo *>::const_iterator  previous = current;

      keepPrograms->push_back(**current);

      ++current;

      for ( ; current != keepProgramList.end(); ++current, ++previous)
      {
         const Time  &previousEnd  = (*previous)->GetEndTime();
         const Time  &currentStart = (*current)->GetStartTime();

         if (previousEnd  < currentStart)
         {
            // We have a gap -- add an empty program
            ProgramInfo noInformation(previousEnd, currentStart, Metrics::GetNoProgrammeText(), Metrics::GetNoProgrammeDescription());

            keepPrograms->push_back(noInformation);
         }

         keepPrograms->push_back(**current);
      }

      const Time  &previousEnd = (*previous)->GetEndTime();

      if (previousEnd < region.GetFinishTime())
      {
         ProgramInfo noInformation(previousEnd, region.GetFinishTime(), Metrics::GetNoProgrammeText(), Metrics::GetNoProgrammeDescription());

         keepPrograms->push_back(noInformation);
      }
   }
}

void ChannelNode::SetKeepRegion(const Region &region)
{
   m_root->ClearChildren();
   m_root->ClearGeometry();
   m_interval.clear();

   // Find all programs that intersect the keep region and sort
   std::list<const ProgramInfo *> keepProgramList;
   GatherPrograms(&keepProgramList, m_info, region);

   // Check the programs in order and insert bogus programs into any gaps
   std::vector<ProgramInfo> keepPrograms;
   keepPrograms.reserve(keepProgramList.size());
   RationalisePrograms(&keepPrograms, keepProgramList, region);

   // Create the first interval
   m_interval.push_back(IntervalNode(keepPrograms[0].GetStartTime()));
   IntervalNode   *interval = &m_interval.back();
   m_root->AppendChild(interval->GetRoot());

   for (uint32_t p = 0; p < keepPrograms.size(); ++p)
   {
      // Do we need a new interval?
      if (!interval->IntervalIncludes(keepPrograms[p].GetStartTime()))
      {
         m_interval.push_back(IntervalNode(keepPrograms[p].GetStartTime()));
         interval = &m_interval.back();
         m_root->AppendChild(interval->GetRoot());
      }

      interval->AddProgram(m_line, keepPrograms[p]);
   }
}


void ChannelNode::HighlightSelection(Selection &selection, bool select)
{
   bool  found = false;

   for (std::list<IntervalNode>::iterator i = m_interval.begin(); !found && i != m_interval.end(); ++i)
   {
      if ((*i).ProgramsInclude(selection.GetTime()))
      {
         found = true;
         (*i).HighlightSelection(selection, select);
      }
   }
}

const Time *ChannelNode::FindNextProgramme(const Time &t) const
{
   const Time *res = 0;

   for (std::list<IntervalNode>::const_iterator i = m_interval.begin(); res == 0 && i != m_interval.end(); ++i)
   {
      const IntervalNode   &node = *i;

      if (node.ProgramsInclude(t))
      {
         res = node.FindNextProgramme(t);

        // Next programme wasn't in the interval, try the next one
         if (res == 0)
         {
            std::list<IntervalNode>::const_iterator next = i;
            ++next;

            if (next != m_interval.end())
               res = (*next).GetFirstProgStartTime();
            else
               res = &node.GetLastProgFinishTime();
         }
      }
   }

   return res;
}

const Time *ChannelNode::FindPreviousProgramme(const Time &t) const
{
   const Time *res = 0;

   for (std::list<IntervalNode>::const_iterator i = m_interval.begin(); res == 0 && i != m_interval.end(); ++i)
   {
      const IntervalNode   &node = *i;

      if (node.ProgramsInclude(t))
      {
         res = node.FindPreviousProgramme(t);

        // Previous programme wasn't in the interval, try the previous one
         if (res == 0)
         {
            if (i != m_interval.begin())
            {
               std::list<IntervalNode>::const_iterator prev = i;
               --prev;

               res = (*prev).GetLastProgStartTime();
            }
         }
      }
   }

   return res;
}

