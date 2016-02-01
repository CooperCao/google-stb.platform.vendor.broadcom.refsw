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

#include "pg.h"
#include "pg_interval_node.h"
#include "pg_info.h"
#include "pg_region.h"

#include <algorithm>

#undef max

using namespace pg;
using namespace bsg;

static Time RoundOff(const Time &tm, uint32_t roundHours)
{
   int32_t hours = (int32_t)floorf(tm.FloatHours() / roundHours) * roundHours;

   return Time(hours, Time::eHOURS);
}

IntervalNode::IntervalNode(const bsg::Time &start) :
   m_root(bsg::New),
   m_start(RoundOff(start, GetIntervalHours())),
   m_lastProgFinish(m_start),
   m_line(0)
{}

void IntervalNode::AddProgram(uint32_t line, const ProgramInfo &prog)
{
   m_line = line;
   // Add program into interval, update the scene graph and the end time for the interval

   PanelNode panel(line, prog);

   m_panel.push_back(panel);
   m_root->AppendChild(panel.GetRoot());

   m_lastProgFinish = std::max(m_lastProgFinish, prog.GetEndTime());
}

void IntervalNode::VisibilityCull(const Region &region)
{
   for (uint32_t i = 0; i < m_panel.size(); ++i)
   {
      PanelNode   &panel = m_panel[i];

      bool  visible = region.IntersectsInterval(panel.GetStartTime(), panel.GetFinishTime());

      panel.GetRoot()->SetVisible(visible);

      if (visible)
         panel.AdjustText(region);
   }
}

/*
static void Dump(const Time &start, const Time &finish, const std::string &msg, const std::vector<PanelNode> &lst)
{
   std::cout << msg << " " << start.Hours() << " to " << finish.Hours() << "\n";

   for (uint32_t i = 0; i  < lst.size(); ++i)
      std::cout << lst[i].GetStartTime().Hours() << " --> " << lst[i].GetFinishTime().Hours() << "\n";
}
*/

void IntervalNode::HighlightSelection(Selection &selection, bool select)
{
   bool  found = false;

   //std::cout << "Interval: " << GetIntervalStartTime().Hours() << "  " << GetIntervalFinishTime().Hours() << "\n";
   for (std::vector<PanelNode>::iterator  i = m_panel.begin(); !found && i != m_panel.end(); ++i)
   {
      PanelNode   &panel = *i;

      if (panel.GetStartTime() <= selection.GetTime() && panel.GetFinishTime() > selection.GetTime())
      {
         found = true;
         panel.HighlightSelection(selection, select);
      }
   }
}

const Time *IntervalNode::FindNextProgramme(const Time &t) const
{
   const Time  *res = 0;

   for (std::vector<PanelNode>::const_iterator  i = m_panel.begin(); res == 0 && i != m_panel.end(); ++i)
   {
      if (t >= (*i).GetStartTime() && t < (*i).GetFinishTime())
      {
         std::vector<PanelNode>::const_iterator  next = i;
         ++next;

         if (next != m_panel.end())
            res = &((*next).GetStartTime());
      }
   }

   return res;
}

const Time *IntervalNode::FindPreviousProgramme(const Time &t) const
{
   const Time  *res = 0;

   for (std::vector<PanelNode>::const_iterator  i = m_panel.begin(); res == 0 && i != m_panel.end(); ++i)
   {
      if (t >= (*i).GetStartTime() && t < (*i).GetFinishTime())
      {
         if (i != m_panel.begin())
         {
            std::vector<PanelNode>::const_iterator  prev = i;
            --prev;

            res = &((*prev).GetStartTime());
         }
      }
   }

   return res;
}

const Time *IntervalNode::GetLastProgStartTime() const
{
   if (m_panel.size() == 0)
      return 0;

   return &m_panel.back().GetStartTime();
}

const Time *IntervalNode::GetFirstProgStartTime() const
{
   if (m_panel.size() == 0)
      return 0;

   return &m_panel.front().GetStartTime();
}

