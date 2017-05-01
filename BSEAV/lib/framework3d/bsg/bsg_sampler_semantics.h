/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef __BSG_SAMPLER_SEMANTICS_H__
#define __BSG_SAMPLER_SEMANTICS_H__

#include <map>

#include "bsg_common.h"
#include "bsg_gl_texture.h"

namespace bsg
{

// @cond
class SamplerSemantics
{
public:
   enum eType
   {
      eUNKNOWN_TYPE = 0,
      eSAMPLER_2D,
#ifdef BSG_USE_ES3
      eSAMPLER_3D,
      eSAMPLER_2D_ARRAY,
#endif
      eSAMPLER_CUBE,
   };

public:
   SamplerSemantics() {}

   void                 SetSemantic(const std::string &str, const GLSamplerState &state);
   const GLSamplerState &GetSemantic(const std::string &str) const;

   const std::map<std::string, GLSamplerState> &GetSemantics() const { return m_semantics; }

   //! Returns the required set of semantics
   //const std::map<std::string, eSemantic> &RequiredSemantics() const { return m_semantics; }

   void Parse(const std::string &semStr, eType type, const std::string &name);

private:
   //static eSemantic MatchSemanticName(const std::string &name);
   void ParseLine(const std::string &semStr);

private:
   std::map<std::string, GLSamplerState>   m_semantics;
};

// @endcond

}

#endif /* __BSG_SAMPLER_SEMANTICS_H__ */
