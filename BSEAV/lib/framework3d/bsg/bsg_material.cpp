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

#include "bsg_material.h"
#include "bsg_effect.h"
#include "bsg_pass.h"
#include "bsg_gl_uniform.h"

using namespace std;

namespace bsg
{

uint32_t UniqueId::m_count = 0;

Material::~Material()
{
   Reset();
}

void Material::Reset()
{
   m_uniformsNeedForcing = true;

   for (map<string, GLUniformBase *>::const_iterator iter = m_uniforms.begin(); iter != m_uniforms.end(); ++iter)
      delete iter->second;

   m_uniforms.clear();
   m_textures.clear();
}

void Material::SetEffect(EffectHandle effect)
{
   Reset();
   m_effect = effect;
}

const EffectSemantics &Material::GetSemanticRequirements(int32_t pass) const
{
   return m_effect->GetPass(pass)->Semantics();
}

void Material::MakeActive(uint32_t passNum) const
{
   Pass *pass = m_effect->GetPass(passNum);

   // Do we need to force install all the uniforms?
   // We do if anyone else has used the effect pass since we last did.
   bool forceInstall = m_uniformsNeedForcing;
   if (this != pass->LastClientMaterial())
      forceInstall = true;

   pass->SetLastClientMaterial(this);

   // Use the program
   GLProgram &prog = pass->Program();
   prog.Use();

   // Install uniforms
   for (map<string, GLUniformBase *>::const_iterator iter = m_uniforms.begin(); iter != m_uniforms.end(); ++iter)
   {
      iter->second->SetProgram(prog);
      iter->second->Install(forceInstall);
   }

   // Install samplers
   for (map<string, GLTextureBinding>::const_iterator iter = m_textures.begin(); iter != m_textures.end(); ++iter)
   {
      iter->second.Select(prog, iter->first);
   }

   m_uniformsNeedForcing = false;
}

uint32_t Material::NumPasses() const
{
   return m_effect->NumPasses();
}

void Material::SetUniformValue(const std::string &name, const float *scalarArray, uint32_t count)
{
   map<string, GLUniformBase *>::const_iterator iter;
   iter = m_uniforms.find(name);
   if (iter == m_uniforms.end())
      iter = m_uniforms.insert(pair<string, GLUniformBase *>(name, new GLUniform1fArray(name))).first;

   dynamic_cast<GLUniform1fArray*>(iter->second)->SetValue(scalarArray, count);
}

void Material::SetUniformValue(const std::string &name, const Vec2 *vecArray, uint32_t count)
{
   map<string, GLUniformBase *>::const_iterator iter;
   iter = m_uniforms.find(name);
   if (iter == m_uniforms.end())
      iter = m_uniforms.insert(pair<string, GLUniformBase *>(name, new GLUniform2fArray(name))).first;

   dynamic_cast<GLUniform2fArray*>(iter->second)->SetValue(vecArray, count);
}

void Material::SetUniformValue(const std::string &name, const Vec3 *vecArray, uint32_t count)
{
   map<string, GLUniformBase *>::const_iterator iter;
   iter = m_uniforms.find(name);
   if (iter == m_uniforms.end())
      iter = m_uniforms.insert(pair<string, GLUniformBase *>(name, new GLUniform3fArray(name))).first;

   dynamic_cast<GLUniform3fArray*>(iter->second)->SetValue(vecArray, count);
}

void Material::SetUniformValue(const std::string &name, const Vec4 *vecArray, uint32_t count)
{
map<string, GLUniformBase *>::const_iterator iter;
iter = m_uniforms.find(name);
if (iter == m_uniforms.end())
iter = m_uniforms.insert(pair<string, GLUniformBase *>(name, new GLUniform4fArray(name))).first;

dynamic_cast<GLUniform4fArray*>(iter->second)->SetValue(vecArray, count);
}

void Material::SetUniformValue(const std::string &name, const std::vector<float> &scalarArray)
{
   map<string, GLUniformBase *>::const_iterator iter;
   iter = m_uniforms.find(name);
   if (iter == m_uniforms.end())
      iter = m_uniforms.insert(pair<string, GLUniformBase *>(name, new GLUniform1fVector(name))).first;

   dynamic_cast<GLUniform1fVector*>(iter->second)->SetValue(scalarArray);
}

void Material::SetUniformValue(const std::string &name, const std::vector<AnimatableFloat> &scalarArray)
{
   map<string, GLUniformBase *>::const_iterator iter;
   iter = m_uniforms.find(name);
   if (iter == m_uniforms.end())
      iter = m_uniforms.insert(pair<string, GLUniformBase *>(name, new GLUniform1fVector(name))).first;

   std::vector<float>   fvec(scalarArray.size());
   for (uint32_t i = 0; i < scalarArray.size(); i++)
      fvec[i] = scalarArray[i];

   dynamic_cast<GLUniform1fVector*>(iter->second)->SetValue(fvec);
}

void Material::SetUniformValue(const std::string &name, const std::vector<Vec2> &vecArray)
{
   map<string, GLUniformBase *>::const_iterator iter;
   iter = m_uniforms.find(name);
   if (iter == m_uniforms.end())
      iter = m_uniforms.insert(pair<string, GLUniformBase *>(name, new GLUniform2fVector(name))).first;

   dynamic_cast<GLUniform2fVector*>(iter->second)->SetValue(vecArray);
}

void Material::SetUniformValue(const std::string &name, const std::vector<AnimatableVec2> &vecArray)
{
   map<string, GLUniformBase *>::const_iterator iter;
   iter = m_uniforms.find(name);
   if (iter == m_uniforms.end())
      iter = m_uniforms.insert(pair<string, GLUniformBase *>(name, new GLUniform2fVector(name))).first;

   std::vector<Vec2> vec(vecArray.size());
   for (uint32_t i = 0; i < vecArray.size(); i++)
      vec[i] = vecArray[i];

   dynamic_cast<GLUniform2fVector*>(iter->second)->SetValue(vec);
}

void Material::SetUniformValue(const std::string &name, const std::vector<Vec3> &vecArray)
{
   map<string, GLUniformBase *>::const_iterator iter;
   iter = m_uniforms.find(name);
   if (iter == m_uniforms.end())
      iter = m_uniforms.insert(pair<string, GLUniformBase *>(name, new GLUniform3fVector(name))).first;

   dynamic_cast<GLUniform3fVector*>(iter->second)->SetValue(vecArray);
}

void Material::SetUniformValue(const std::string &name, const std::vector<AnimatableVec3> &vecArray)
{
   map<string, GLUniformBase *>::const_iterator iter;
   iter = m_uniforms.find(name);
   if (iter == m_uniforms.end())
      iter = m_uniforms.insert(pair<string, GLUniformBase *>(name, new GLUniform3fVector(name))).first;

   std::vector<Vec3> vec(vecArray.size());
   for (uint32_t i = 0; i < vecArray.size(); i++)
      vec[i] = vecArray[i];

   dynamic_cast<GLUniform3fVector*>(iter->second)->SetValue(vec);
}

void Material::SetUniformValue(const std::string &name, const std::vector<Vec4> &vecArray)
{
   map<string, GLUniformBase *>::const_iterator iter;
   iter = m_uniforms.find(name);
   if (iter == m_uniforms.end())
      iter = m_uniforms.insert(pair<string, GLUniformBase *>(name, new GLUniform4fVector(name))).first;

   dynamic_cast<GLUniform4fVector*>(iter->second)->SetValue(vecArray);
}

void Material::SetUniformValue(const std::string &name, const std::vector<AnimatableVec4> &vecArray)
{
   map<string, GLUniformBase *>::const_iterator iter;
   iter = m_uniforms.find(name);
   if (iter == m_uniforms.end())
      iter = m_uniforms.insert(pair<string, GLUniformBase *>(name, new GLUniform4fVector(name))).first;

   std::vector<Vec4> vec(vecArray.size());
   for (uint32_t i = 0; i < vecArray.size(); i++)
      vec[i] = vecArray[i];

   dynamic_cast<GLUniform4fVector*>(iter->second)->SetValue(vec);
}

const GLSamplerState *Material::FindSamplerState(const string &name) const
{
   for (uint32_t pass = 0; pass < NumPasses(); ++pass)
   {
      const SamplerSemantics  &semantics = m_effect->GetPass(pass)->GetSamplerSemantics();

      const std::map<std::string, GLSamplerState>  &list = semantics.GetSemantics();

      std::map<std::string, GLSamplerState>::const_iterator iter = list.find(name);

      if (iter != list.end())
         return &(*iter).second;
   }

   return 0;
}

void Material::SetTexture(const string &name, TextureHandle texture)
{
   map<string, GLTextureBinding>::iterator iter;
   iter = m_textures.find(name);
   if (iter == m_textures.end())
   {
      // Bind the texture handle to its semantics
      const GLSamplerState *state = FindSamplerState(name);

      if (state == 0)
         BSG_THROW("Texture sampler '" << name << "' not found in effect file");

      m_textures.insert(pair<string, GLTextureBinding>(name, GLTextureBinding(texture, state)));
   }
   else
   {
      iter->second.SetTexture(texture);
   }
}

AnimTarget<TextureHandle> &Material::GetTexture(const std::string &name)
{
   map<string, GLTextureBinding>::iterator iter;
   iter = m_textures.find(name);
   if (iter == m_textures.end())
   {
      // Bind the texture handle to its semantics
      const GLSamplerState *state = FindSamplerState(name);

      if (state == 0)
         BSG_THROW("Texture sampler '" << name << "' not found in effect file");

      iter = m_textures.insert(pair<string, GLTextureBinding>(name, GLTextureBinding(TextureHandle(), state))).first;
   }

   return iter->second;
}

}
