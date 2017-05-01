/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef __BSG_PASS_H__
#define __BSG_PASS_H__

#include "bsg_common.h"
#include "bsg_gl_program.h"
#include "bsg_pass_state.h"
#include "bsg_effect_semantics.h"
#include "bsg_sampler_semantics.h"

namespace bsg
{

class Material;

// @cond
class Pass
{
public:
   Pass() : m_lastClient(NULL) {}

   //! Program and semantics have been built -- evaluate info
   void CacheSemantics();

   PassState         &State()       { return m_state;       }
   EffectSemantics   &Semantics()   { return m_semantics;   }
   SamplerSemantics  &Samplers()    { return m_samplers;    }
   GLProgram         &Program()     { return m_program;     }

   const Material    *LastClientMaterial() const                { return m_lastClient; }
   void              SetLastClientMaterial(const Material *mat) { m_lastClient = mat;  }

   const AttributeSemantics &GetAttributeSemantics() const { return m_attributeSemantics; }
   const SamplerSemantics   &GetSamplerSemantics()   const { return m_samplers;           }

private:
   PassState         m_state;          // Render state
   EffectSemantics   m_semantics;      // Effect semantics
   GLProgram         m_program;        // Vertex & Fragment shader

   const Material    *m_lastClient;    // Last material that used this pass

   AttributeSemantics m_attributeSemantics;
   SamplerSemantics   m_samplers;      // Sampler states
};
// @endcond

}

#endif /* __BSG_PASS_H__ */
