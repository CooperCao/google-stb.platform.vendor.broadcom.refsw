/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
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
