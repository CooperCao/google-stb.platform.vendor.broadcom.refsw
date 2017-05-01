/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef __PG_INTERVAL_NODE_H__
#define __PG_INTERVAL_NODE_H__

#include "pg_panel.h"
#include "bsg_time.h"

namespace pg
{

class ProgramInfo;
class Region;
class Selection;

class IntervalNode
{
public:
   IntervalNode() :
      m_root(bsg::New),
      m_line(0)
   {}

   IntervalNode(const bsg::Time &start);

   static uint32_t GetIntervalHours() { return 4; }
   static bsg::Time GetIntervalTime() { return bsg::Time(GetIntervalHours(), bsg::Time::eHOURS); }

   bool IntervalIncludes(const bsg::Time &t) const
   {
      return t >= m_start && t < (m_start + GetIntervalTime());
   }

   bool ProgramsInclude(const bsg::Time &t) const
   {
      return t >= m_start && t < m_lastProgFinish;
   }

   void AddProgram(uint32_t line, const ProgramInfo &program);
   uint32_t NumPrograms() const { return m_panel.size(); }

   const bsg::SceneNodeHandle &GetRoot() const  { return m_root; }
   bsg::SceneNodeHandle GetRoot()               { return m_root; }

   const bsg::Time &GetIntervalStartTime() const  { return m_start;                      }
   const bsg::Time GetIntervalFinishTime() const  { return m_start + GetIntervalTime();  }

   const bsg::Time &GetLastProgFinishTime() const { return m_lastProgFinish;             }

   void VisibilityCull(const Region &region);
   void HighlightSelection(Selection &selection, bool select);
   void Sort(bsg::Time *previousEnd);

   const bsg::Time   *FindNextProgramme(const bsg::Time &t) const;
   const bsg::Time   *FindPreviousProgramme(const bsg::Time &t) const;
   const bsg::Time   *GetLastProgStartTime() const;
   const bsg::Time   *GetFirstProgStartTime() const;

private:
   bsg::SceneNodeHandle    m_root;
   bsg::Time               m_start;
   bsg::Time               m_lastProgFinish;
   std::vector<PanelNode>  m_panel;
   uint32_t                m_line;
};

}

#endif
