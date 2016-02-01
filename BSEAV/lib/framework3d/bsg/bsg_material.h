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

#ifndef __BSG_MATERIAL_H__
#define __BSG_MATERIAL_H__

#include "bsg_common.h"
#include "bsg_effect_semantics.h"
#include "bsg_effect.h"
#include "bsg_trackers.h"

#include <stdint.h>
#include <string>
#include <map>

#include "bsg_gl.h"
#include "bsg_gl_uniform.h"
#include "bsg_library.h"
#include "bsg_animatable.h"

namespace bsg
{

class Material;

class UniqueId
{
public:
   UniqueId() :
      m_id(m_count++)
   {}

   uint32_t Get() const { return m_id; }

private:
   uint32_t          m_id;

   static uint32_t   m_count;
};

// @cond
struct MaterialTraits
{
   typedef Material        Base;
   typedef Material        Derived;
   typedef MaterialTraits  BaseTraits;
};
// @endcond

//! @addtogroup handles
//! @{
typedef Handle<MaterialTraits>   MaterialHandle;
//! @}

typedef std::vector<MaterialHandle> Materials;

class Vec2;
class Vec3;
class Vec4;
class IVec2;
class IVec3;
class IVec4;
class Mat2;
class Mat3;
class Mat4;

////////////////////////////////////////////////////////////////////////////

//! @addtogroup scenegraph 
//! @{

//! A Material is an instantiation of a Effect with its uniforms appropriately set
//! for the material. For example, a 'plastic' effect might be instantiated as a 'blue shiny plastic' material.
//! 
//! A Effect can be shared by multiple Material objects.
//! 
//! Some types of uniform (integer and float scalars and vectors) can be accessed as animatable values, so
//! that they can be treated as standard animatable components.
//! 
//! This is how you might create a material and set some uniforms and textures:
//! \code
//! using namespace bsg;
//! 
//! MaterialHandle mat(New);
//! EffectHandle   effect(New);
//! 
//! std::ifstream  is((GetOptions().GetResourcePath() + "my_effect.bfx").c_str());
//! effect->Load(is);
//! 
//! mat->SetEffect(effect);
//! mat->SetTexture("u_tex", irrad);
//! mat->SetTexture("u_refl_tex", refl);
//! mat->SetTexture("u_occlusion", occlusion);
//! mat->SetUniformValue("u_color", color);
//! mat->SetUniformValue("u_dimScale", dimScale);
//! \endcode
class Material : public RefCount
{
   friend class Handle<MaterialTraits>;

public:
   virtual ~Material();

   //! Set the bsg::Effect that will be used by this material
   void         SetEffect(EffectHandle effect);
   //! Get the bsg::Effect used by this material
   EffectHandle GetEffect() const { return m_effect; }

   //! Return the number of passes needed by the bsg::Effect in this material
   uint32_t NumPasses() const;

   //! Return's this objects unique identifier
   uint32_t GetId() const { return m_id.Get(); }

   // @cond
   const EffectSemantics    &GetSemanticRequirements(int32_t pass) const;
   const AttributeSemantics &GetAttributeSemantics(uint32_t pass)  const  { return m_effect->GetPass(pass)->GetAttributeSemantics();   }
   const SamplerSemantics   &GetSamplerSemantics(uint32_t pass)    const  { return m_effect->GetPass(pass)->GetSamplerSemantics();     }
   // @endcond

   //! Makes this material's state active for rendering the given pass
   void MakeActive(uint32_t pass) const;

   //! @name Uniform Setting
   //! Uniforms are referenced by their name, as defined in the SEMANTICS block of the effect file.
   //! SetUniformValue has overrides for all relevant types of uniform
   //! SetUniform is a templated version (used to implement the non-array overloads).

   //@{ 
private:
   // This is effectively a look-up table of controlled type and its corresponding Uniform type.
   // Note that matrices are not currently animatable, so they do not have animatable types.
   /*
   template <typename T> struct MaterialUniformType          {                                     };
   template <>           struct MaterialUniformType<GLint>   { typedef AnimatableUniform1i   Type; };
   template <>           struct MaterialUniformType<IVec2>   { typedef AnimatableUniform2i   Type; };
   template <>           struct MaterialUniformType<IVec3>   { typedef AnimatableUniform3i   Type; };
   template <>           struct MaterialUniformType<IVec4>   { typedef AnimatableUniform4i   Type; };
   template <>           struct MaterialUniformType<GLfloat> { typedef AnimatableUniform1f   Type; };
   template <>           struct MaterialUniformType<Vec2>    { typedef AnimatableUniform2f   Type; };
   template <>           struct MaterialUniformType<Vec3>    { typedef AnimatableUniform3f   Type; };
   template <>           struct MaterialUniformType<Vec4>    { typedef AnimatableUniform4f   Type; };
   template <>           struct MaterialUniformType<Mat2>    { typedef AnimatableUniformMat2 Type; };
   template <>           struct MaterialUniformType<Mat3>    { typedef GLUniformMat3         Type; };
   template <>           struct MaterialUniformType<Mat4>    { typedef GLUniformMat4         Type; };
   */

public:
   // Generic implementation of SetUniform for non-array types
   template <class T>
   void SetUniform(const std::string &name, const T &v)
   {
      typedef AnimatableUniform<T> Type;

      auto iter = m_uniforms.find(name);
      if (iter == m_uniforms.end())
         iter = m_uniforms.insert(std::pair<std::string, GLUniformBase *>(name, new Type(name))).first;

      dynamic_cast<Type *>(iter->second)->SetValue(v);
   }

   void SetUniformValue(const std::string &name, float scalar)    { SetUniform(name, scalar);   }
   void SetUniformValue(const std::string &name, GLint scalar)    { SetUniform(name, scalar);   }
   void SetUniformValue(const std::string &name, const Vec2 &v)   { SetUniform(name, v);        }
   void SetUniformValue(const std::string &name, const Vec3 &v)   { SetUniform(name, v);        }
   void SetUniformValue(const std::string &name, const Vec4 &v)   { SetUniform(name, v);        }
   void SetUniformValue(const std::string &name, const IVec2 &v)  { SetUniform(name, v);        }
   void SetUniformValue(const std::string &name, const IVec3 &v)  { SetUniform(name, v);        }
   void SetUniformValue(const std::string &name, const IVec4 &v)  { SetUniform(name, v);        }
   void SetUniformValue(const std::string &name, const Mat2 &m)   { SetUniform(name, m);        }
   void SetUniformValue(const std::string &name, const Mat3 &m)   { SetUniform(name, m);        }
   void SetUniformValue(const std::string &name, const Mat4 &m)   { SetUniform(name, m);        }

   void SetUniformValue(const std::string &name, const float *scalarArray, uint32_t count);
   void SetUniformValue(const std::string &name, const Vec2 *vecArray, uint32_t count);
   void SetUniformValue(const std::string &name, const Vec3 *vecArray, uint32_t count);
   void SetUniformValue(const std::string &name, const Vec4 *vecArray, uint32_t count);
   
   void SetUniformValue(const std::string &name, const std::vector<float> &scalarArray);
   void SetUniformValue(const std::string &name, const std::vector<Vec2> &vecArray);
   void SetUniformValue(const std::string &name, const std::vector<Vec3> &vecArray);
   void SetUniformValue(const std::string &name, const std::vector<Vec4> &vecArray);
   void SetUniformValue(const std::string &name, const std::vector<AnimatableFloat> &scalarArray);
   void SetUniformValue(const std::string &name, const std::vector<AnimatableVec2> &vecArray);
   void SetUniformValue(const std::string &name, const std::vector<AnimatableVec3> &vecArray);
   void SetUniformValue(const std::string &name, const std::vector<AnimatableVec4> &vecArray);
   //@}

   //! @name Uniform Getting
   //! Uniforms are referenced by their name, as defined in the SEMANTICS block of the effect file.
   //! GetUniform<T> has instantiations for all relevant types of animatable uniform (T = float, int, VecN, IVecN)
   //! GetUniformNt provides convenience names for animatable uniforms size N type t e.g. GetUniform1f.
   //! Getting a uniform returns a reference to the uniform.  Use Get() and Set() on this reference to
   //! modify the value.
  
   //@{
   // Generic GetUniform
   template <class T>
   AnimatableUniform<T> &GetUniform(const std::string &name)
   {
      typedef AnimatableUniform<T> Type;

      auto iter = m_uniforms.find(name);
      if (iter == m_uniforms.end())
         iter = m_uniforms.insert(std::pair<std::string, GLUniformBase *>(name, new Type(name))).first;

      Type  *res = dynamic_cast<Type *>(iter->second);

      if (res == nullptr)
         BSG_THROW("Type mismatch getting uniform");

      return *res;
   }

   AnimatableUniform1f &GetUniform1f(const std::string &name) { return GetUniform<float>(name); }
   AnimatableUniform2f &GetUniform2f(const std::string &name) { return GetUniform<Vec2>(name);  }
   AnimatableUniform3f &GetUniform3f(const std::string &name) { return GetUniform<Vec3>(name);  }
   AnimatableUniform4f &GetUniform4f(const std::string &name) { return GetUniform<Vec4>(name);  }
   AnimatableUniform1i &GetUniform1i(const std::string &name) { return GetUniform<int>(name);   }
   AnimatableUniform2i &GetUniform2i(const std::string &name) { return GetUniform<IVec2>(name); }
   AnimatableUniform3i &GetUniform3i(const std::string &name) { return GetUniform<IVec3>(name); }
   AnimatableUniform4i &GetUniform4i(const std::string &name) { return GetUniform<IVec4>(name); }
   //@}

   //! Set the texture object to associated with the named texture in the effect
   void SetTexture(const std::string &name, TextureHandle t);
   AnimTarget<TextureHandle> &GetTexture(const std::string &name);

   //! Returns the GL state manipulator for a particular render pass
   const PassState &GetGLState(uint32_t pass) const { return m_effect->GetPass(pass)->State(); }

protected:
   Material() :
      m_uniformsNeedForcing(true)
   {}

private:
   const GLSamplerState *FindSamplerState(const std::string &name) const;
   void Reset();

private:
   EffectHandle                              m_effect;

   std::map<std::string, GLUniformBase *>    m_uniforms;
   std::map<std::string, GLTextureBinding >  m_textures;

   UniqueId                                  m_id;

   mutable bool                              m_uniformsNeedForcing;
};

class MaterialMap
{
public:
   void Append(const std::string &name, const MaterialHandle handle)
   {
      m_map[name] = handle;
   }

   MaterialHandle Find(const std::string &name) const
   {
      auto entry = m_map.find(name);

      if (entry == m_map.end())
         BSG_THROW("Cannot find material '" << name << "'");

      return entry->second;
   }

private:
   std::map<std::string, MaterialHandle>  m_map;
};

//! @}

}

#endif /* __BSG_MATERIAL_H__ */

