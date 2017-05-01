/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef __PG_CHANNEL_NODE_H__
#define __PG_CHANNEL_NODE_H__

#include "pg_interval_node.h"

#include "bsg_scene_node.h"

namespace pg
{

class ChannelInfo;
class ProgramInfo;
class Selection;
class Region;

class ChannelNode
{
public:
   ChannelNode();
   ChannelNode(uint32_t lineNo, const ChannelInfo &info);

   const bsg::SceneNodeHandle &GetRoot() const   { return m_root;  }
   bsg::SceneNodeHandle GetRoot()                { return m_root;  }

   IntervalNode   &FindInterval(const ProgramInfo &prog);

   void VisibilityCull(const Region &region);
   void SetKeepRegion(const Region &region);
   void HighlightSelection(Selection &selection, bool select);

   const bsg::Time *FindNextProgramme(const bsg::Time &t)      const;
   const bsg::Time *FindPreviousProgramme(const bsg::Time &t)  const;

   bool IsValid() const { return m_name != ""; }

private:
   void SortIntervals(const Region &region);

private:
   bsg::SceneNodeHandle       m_root;
   std::string                m_name;
   std::list<IntervalNode>    m_interval;
   uint32_t                   m_line;
   bool                       m_empty;
   const ChannelInfo          *m_info;
};

}

#endif
