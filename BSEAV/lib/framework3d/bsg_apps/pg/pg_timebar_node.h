/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef __PG_TIMEBAR_NODE_H__
#define __PG_TIMEBAR_NODE_H__

#include "pg_interval_node.h"

#include "bsg_scene_node.h"

#include <vector>

namespace pg
{

class ChannelInfo;
class ProgramInfo;
class Region;

class TimeBarNode
{
public:
   TimeBarNode();

   const bsg::SceneNodeHandle &GetRoot() const   { return m_root;  }
   bsg::SceneNodeHandle GetRoot()                { return m_root;  }

   void SetKeepRegion(const Region &region);
   void VisibilityCull(const Region &region);

private:
   bsg::GeometryHandle GetTextGeometry(const bsg::Time &t);
   void AppendTickGeometry(bsg::SceneNodeHandle &node, const bsg::Time &t);

private:
   bsg::SceneNodeHandle       m_root;
   bsg::PrintFontHandle       m_font;
   bsg::EffectHandle          m_textEffect;
   bsg::EffectHandle          m_tickEffect;
   bsg::EffectHandle          m_lineEffect;
   bsg::MaterialHandle        m_tickMaterial;
   bsg::MaterialHandle        m_lineMaterial;
   bsg::Time                  m_firstNodeTime;
   std::vector<bsg::Time>     m_nodeTimes;
};

}

#endif
