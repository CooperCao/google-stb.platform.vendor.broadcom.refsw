/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef __PG_CONTROLLER_H__
#define __PG_CONTROLLER_H__

#include "pg_channel_node.h"
#include "pg_interval_node.h"
#include "pg_timebar_node.h"
#include "pg_channelbar_node.h"
#include "pg_region.h"

namespace pg
{

class GridInfo;
class Selection;
class SceneNodeCallback;

//! This class is responsible for maintaining the scene graph, and for controlling
//! the visibility of nodes.

class Controller
{
public:
   Controller();

   void Initialise(const GridInfo &info, const Region &activeRegion, const Region &keepRegion);

   const bsg::SceneNodeHandle &GetRoot() const        { return m_root;                 }
   bsg::SceneNodeHandle &GetLongScrollingNode()       { return m_longScrolling2d; }
   bsg::AnimTarget<bsg::Transform>  &GetTransform3D() { return m_3d->GetTransform();   }

   void UpdateAll(const Region &activeRegion, const Region &keepRegion);

   void SetActiveRegion(const Region &region);
   void SetKeepRegion(const Region &region);
   void VisibilityCull(const Region &region);
   void HighlightSelection(Selection &selecton, bool highlight = true);

   const bsg::Time   *FindNextProgramme(Selection &selecton) const;
   const bsg::Time   *FindPreviousProgramme(Selection &selecton) const;

   void UpdateFrame();

   bsg::AnimatableGroup<bsg::Quaternion>  &GetPanelRotation() { return m_panelRotGroup; }

   void SetProgramsNodeCallBack(bsg::SceneNodeCallback *callBack);
   void DisplayLongScrolling(bool longScrollingOn);

private:
   void Clean();
   void UpdatePastRectangle();

private:
   bsg::SceneNodeHandle       m_root;
   bsg::SceneNodeHandle       m_3d;
   bsg::SceneNodeHandle       m_2d;
   bsg::SceneNodeHandle       m_time2d;
   bsg::SceneNodeHandle       m_channel2d;
   bsg::SceneNodeHandle       m_pastMarker;
   bsg::MaterialHandle        m_pastMaterial;
   bsg::SceneNodeHandle       m_longScrolling2d;
   bsg::Tasker                m_tasker;

   bsg::AnimatableGroup<bsg::Quaternion> m_panelRotGroup;

   std::vector<ChannelNode>   m_channelRoot;
   TimeBarNode                m_timebarNode;
   ChannelBarNode             m_channelbarNode;

   float                      m_timebarHeight;
   float                      m_channelLabelLength;
   Region                     m_activeRegion;

   const GridInfo             *m_info;
};

}

#endif
