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

#ifndef __PG_PANEL_H__
#define __PG_PANEL_H__

#include "pg_panel.h"
#include "pg_info.h"

#include "bsg_time.h"
#include "bsg_geometry.h"
#include "bsg_print.h"
#include "bsg_application.h"
#include "bsg_animation_sequence.h"

#include <string>
#include <map>
#include <memory>

namespace pg
{

class ProgramInfo;
class Region;
class Selection;

class PanelCache
{
public:
   typedef float                                      Width;
   typedef uint32_t                                   Line;

   typedef std::pair<bsg::Time, std::string>          TextKey;
   typedef std::map<TextKey, bsg::PrintHandle>        TextMap;

   typedef Width                                      SurfaceKey;
   typedef std::map<SurfaceKey, bsg::SurfaceHandle>   SurfaceMap;

   typedef std::pair<Line, Width>                     GeometryKey;
   typedef std::map<GeometryKey, bsg::GeometryHandle> GeometryMap;

   PanelCache();
   ~PanelCache();

   bsg::GeometryHandle GetPanelGeometry(uint32_t line, const ProgramInfo &prog);
   bsg::PrintHandle    GetTextGeometry(uint32_t line, const ProgramInfo &prog, const TextKey &key);
   bsg::PrintHandle    GetTextGeometry(uint32_t line, const bsg::Time &start, const bsg::Time &finish, const std::string &title, float offset, const TextKey &key);
   bsg::GeometryHandle GetHighlightGeometry(const bsg::Time &start, const bsg::Time &finish);

   static PanelCache *GetPanelCache();

   bsg::PrintFontHandle &GetFont() { return m_font; }
   void Clear();

private:
   SurfaceMap              m_surfaceMap;     // Maps time in minutes to its panel surface
   GeometryMap             m_geometryMap;    // Maps line and time in minutes to its panel geometry
   TextMap                 m_textMap;
   bsg::PrintFontHandle    m_font;
   bsg::PrintOptions       m_printOpt;
   bsg::EffectHandle       m_textEffect;
   bsg::EffectHandle       m_glowEffect;
   bsg::MaterialHandle     m_glowMaterial;
   bsg::TextureHandle      m_glowTexture;
   bsg::AnimationSequence  m_glowSequence;
};

class PanelNode
{
public:
   PanelNode(uint32_t line, const ProgramInfo &prog);
   ~PanelNode();

   const bsg::SceneNodeHandle &GetRoot() const { return m_root; }
   bsg::SceneNodeHandle &GetRoot() { return m_root; }

   const bsg::Time &GetStartTime() const  { return m_prog.GetStartTime();  }
   const bsg::Time &GetFinishTime() const { return m_prog.GetEndTime();    }
   const std::string GetTitle() const     { return m_prog.GetTitle();      }

   void HighlightSelection(Selection &selection, bool select);
   bool IsSelected() const;

   void AdjustText(const Region &activeRegion);

private:
   bsg::SceneNodeHandle    m_root;
   bsg::SceneNodeHandle    m_textNode;
   ProgramInfo             m_prog;
   bool                    m_shifted;
   uint32_t                m_line;
   float                   m_textSize;
   bool                    m_hasShortened;
   PanelCache::TextKey     m_textKey;
};

}

#endif
