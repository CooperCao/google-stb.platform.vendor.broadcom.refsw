/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
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
