/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "bsg_surface.h"
#include "bsg_gl_buffer.h"
#include "bsg_material.h"
#include "bsg_exception.h"
#include "bsg_material.h"
#include "bsg_effect_semantics.h"

#include <vector>
using namespace std;

namespace bsg
{

// Surface methods
void Surface::AttributesBegin(uint32_t pass, Material *material) const
{
#ifdef BSG_USE_ES3
   if (m_vao == 0)
      glGenVertexArrays(1, &m_vao);

   glBindVertexArray(m_vao);

   // Check if the material is the same as last time
   if (m_lastMaterial != NULL && material->GetId() == m_lastMaterial->GetId())
      return;

   m_lastMaterial = material;
#endif

   const AttributeSemantics &attributes = material->GetAttributeSemantics(pass);

   m_vertexBuffer.Bind();

   for (AttributeSemantics::const_iterator i = attributes.begin(); i != attributes.end(); ++i)
   {
      EffectSemantics::eSemantic   semantic = (*i).Semantic();
      GLuint                       index    = (*i).Index();

      if (GetPointer(semantic).Pointer(index))
         glEnableVertexAttribArray(index);
   }
}

void Surface::AttributesEnd(uint32_t pass, Material *material) const
{
#ifdef BSG_USE_ES3
   glBindVertexArray(0);
#else
   const AttributeSemantics &attributes = material->GetAttributeSemantics(pass);

   for (AttributeSemantics::const_iterator i = attributes.begin(); i != attributes.end(); ++i)
      glDisableVertexAttribArray((*i).Index());
#endif
}

void Surface::UniformsBegin(uint32_t pass, const SemanticData &data, Material *material) const
{
   // Fill out any predefined semantic uniforms that we require
   const EffectSemantics &sem = material->GetSemanticRequirements(pass);

   map<string, EffectSemantics::eSemantic>::const_iterator iter = sem.RequiredSemantics().begin();
   while (iter != sem.RequiredSemantics().end())
   {
      switch (iter->second)
      {
      case EffectSemantics::eMATRIX4_MODEL :
      case EffectSemantics::eMATRIX4_VIEW :
      case EffectSemantics::eMATRIX4_PROJECTION :
      case EffectSemantics::eMATRIX4_MODEL_VIEW :
      case EffectSemantics::eMATRIX4_VIEW_PROJECTION :
      case EffectSemantics::eMATRIX4_MODEL_VIEW_PROJECTION :
         material->SetUniformValue(iter->first, data.GetMat4(iter->second));
         break;
      case EffectSemantics::eMATRIX3_INVT_MODEL :
      case EffectSemantics::eMATRIX3_INVT_VIEW :
      case EffectSemantics::eMATRIX3_INVT_MODEL_VIEW :
         material->SetUniformValue(iter->first, data.GetMat3(iter->second));
         break;
      case EffectSemantics::eSCALAR_OPACITY :
         material->SetUniformValue(iter->first, data.GetFloat(iter->second));
         break;
      case EffectSemantics::eVECTOR4_SCREEN_SIZE :
         material->SetUniformValue(iter->first, data.GetVec4(iter->second));
         break;
      default :
         break;
      }
      ++iter;
   }

   // Ensure all the uniforms (including the ones we've just set are installed now)
   material->MakeActive(pass);
}

// ElementSurface methods
void Surface::Draw(uint32_t pass, Material *material, const SemanticData &data) const
{
   AttributesBegin(pass, material);
   UniformsBegin(pass, data, material);

   if (m_indexBuffer)
   {
      m_indexBuffer->Bind();
#ifdef BSG_USE_ES3
      glDrawElementsInstanced(GetMode(), GetNumVertices(), GL_UNSIGNED_SHORT, 0, m_numInstances); // TODO this assumes 0 offset
#else
      glDrawElements(GetMode(), GetNumVertices(), GL_UNSIGNED_SHORT, 0); // TODO this assumes 0 offset
#endif
   }
   else
   {
#ifdef BSG_USE_ES3
      glDrawArraysInstanced(GetMode(), 0, GetNumVertices(), m_numInstances);
#else
      glDrawArrays(GetMode(), 0, GetNumVertices());
#endif
   }
   AttributesEnd(pass, material);
}

}
