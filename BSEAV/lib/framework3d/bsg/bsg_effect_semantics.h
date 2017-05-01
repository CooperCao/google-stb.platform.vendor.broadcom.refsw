/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef __BSG_EFFECT_SEMANTICS_H__
#define __BSG_EFFECT_SEMANTICS_H__

#include "bsg_common.h"

#include <map>
#include <string>
#include <vector>
#include <stdint.h>

namespace bsg
{

// @cond

class EffectSemantics
{
public:
   enum eSemantic
   {
      eUNKNOWN_SEMANTIC = 0,
      eMATRIX4_MODEL,
      eMATRIX4_VIEW,
      eMATRIX4_PROJECTION,
      eMATRIX4_MODEL_VIEW,
      eMATRIX4_VIEW_PROJECTION,
      eMATRIX4_MODEL_VIEW_PROJECTION,
      eMATRIX3_INVT_MODEL,
      eMATRIX3_INVT_VIEW,
      eMATRIX3_INVT_MODEL_VIEW,
      eSCALAR_OPACITY,
      eSCALAR_USER,
      eVECTOR2_USER,
      eVECTOR3_USER,
      eVECTOR4_USER,
      eMATRIX2_USER,
      eMATRIX3_USER,
      eMATRIX4_USER,
#ifdef BSG_USE_ES3
      eMATRIX2x3_USER,
      eMATRIX2x4_USER,
      eMATRIX3x2_USER,
      eMATRIX3x4_USER,
      eMATRIX4x2_USER,
      eMATRIX4x3_USER,
#endif
      eVECTOR4_SCREEN_SIZE,
      eVECTOR4_QUAD_OFFSET,

      eVATTR_POSITION,
      eVATTR_NORMAL,
      eVATTR_TANGENT,
      eVATTR_BINORMAL,
      eVATTR_TEXCOORD1,
      eVATTR_TEXCOORD2,
      eVATTR_TEXCOORD3,
      eVATTR_COLOR,
      eVATTR_USER1,
      eVATTR_USER2,
      eVATTR_USER3,
      eVATTR_USER4,
      eVATTR_USER5,
      eVATTR_USER6,

      eVATTR_FIRST = eVATTR_POSITION,
      eVATTR_LAST  = eVATTR_USER6,
      eVATTR_COUNT = eVATTR_LAST - eVATTR_FIRST + 1
   };

public:
   EffectSemantics();

   void SetSemantic(const std::string &str, eSemantic sem);
   eSemantic GetSemantic(const std::string &str) const;

   //! Returns the required set of semantics
   const std::map<std::string, eSemantic> &RequiredSemantics() const { return m_semantics; }

   void Parse(const std::string &semStr);

private:
   static eSemantic MatchSemanticName(const std::string &name);
   void ParseLine(const std::string &semStr);

private:
   std::map<std::string, eSemantic>        m_semantics;

   static bool                             m_parseMapInited;
   static std::map<std::string, eSemantic> m_parseMap;
};

class AttributeSemantic
{
public:
   AttributeSemantic() :
      m_semantic(EffectSemantics::eUNKNOWN_SEMANTIC),
      m_index(0)
   {}

   AttributeSemantic(EffectSemantics::eSemantic s, uint32_t index) :
      m_semantic(s),
      m_index(index)
   {}

   EffectSemantics::eSemantic Semantic() const { return m_semantic;  }
   uint32_t                   Index()    const { return m_index;     }

private:
   EffectSemantics::eSemantic m_semantic;
   uint32_t                   m_index;
};

std::string ToString(EffectSemantics::eSemantic semantic);

// @endcond

typedef std::vector<AttributeSemantic>   AttributeSemantics;


}

#endif /* __BSG_EFFECT_SEMANTICS_H__ */
