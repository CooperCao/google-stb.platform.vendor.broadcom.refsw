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
