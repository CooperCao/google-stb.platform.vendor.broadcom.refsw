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
