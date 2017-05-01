/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef __PG_CHANNELBAR_NODE_H__
#define __PG_CHANNELBAR_NODE_H__

#include "pg_interval_node.h"
#include "pg_info.h"

#include "bsg_scene_node.h"

namespace pg
{

class ChannelInfo;
class ProgramInfo;
class Region;

class ChannelBarNode
{
public:
   ChannelBarNode();

   void SetInfo(const GridInfo &info);

   const bsg::SceneNodeHandle &GetRoot() const   { return m_root;  }
   bsg::SceneNodeHandle GetRoot()                { return m_root;  }

   void SetKeepRegion(const Region &region);
   void VisibilityCull(const Region &region);

private:
   bsg::GeometryHandle GetTextGeometry(const std::string &name);

private:
   bsg::SceneNodeHandle       m_root;
   bsg::SceneNodeHandle       m_allNodes;
   bsg::PrintFontHandle       m_font;
   bsg::EffectHandle          m_textEffect;
   bsg::Time                  m_firstNodeTime;
};

}

#endif
