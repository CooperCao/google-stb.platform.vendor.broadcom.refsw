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
