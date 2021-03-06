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

#ifndef __BSG_EFFECT_GENERATOR_H__
#define __BSG_EFFECT_GENERATOR_H__

#include "bsg_common.h"
#include "bsg_material.h"
#include "bsg_obj_reader.h"

#include <map>
#include <string>
#include <iostream>
#include <vector>

//! @cond

namespace bsg
{

///////////////////////////////////////////////////////////////////////////////
// CLASSES
///////////////////////////////////////////////////////////////////////////////

class EffectGenerator
{
public:
   EffectGenerator(const EffectOptions &options, const std::vector<PassState> &states) :
      m_options(options),
      m_states(states),
      m_variables(states.size()),
      m_varyings(states.size())
   {}

   enum eUsage
   {
      eVERTEX               = 1,
      eFRAGMENT             = 2,
      eVERTEX_AND_FRAGMENT  = 3
   };

   void Register(const std::string &name, EffectSemantics::eSemantic, eUsage usage, uint32_t pass = 0);
   void Register(const std::string &name, const std::string &type, EffectSemantics::eSemantic, eUsage usage, uint32_t pass = 0);
   void RegisterSampler(const std::string &name, const std::string &type, eUsage usage, uint32_t pass = 0);
   void Varying(const std::string &name, const std::string &type, uint32_t pass = 0);

   void Generate(std::ostream &os);
   void Indent() const;
   void Begin(const std::string &label);
   void Begin();
   void End(bool extraNewline = true);
   void DeclareSemantics() const;
   void DeclareVert(uint32_t p);
   void DeclareFrag(uint32_t p);
   template <class T> void Print(const T &data) const;
   template <class T> void PrintLn(const T &data) const;
   void PrintLines(const std::vector<std::string> &lines) const;

   virtual  void Options();
   virtual  void Passes();
   virtual  void Pass(uint32_t p);
   virtual  void Semantics(uint32_t p);
   virtual  void State(uint32_t p);
   virtual  void Samplers(uint32_t p);
   virtual  void VertexShader(uint32_t p)    = 0;
   virtual  void FragmentShader(uint32_t p)  = 0;

   class Variable
   {
   public:
      Variable(const std::string &name, const std::string &type, EffectSemantics::eSemantic semantic, eUsage usage);
      Variable(const std::string &name, EffectSemantics::eSemantic semantic, eUsage usage);
      Variable(const std::string &name, const std::string &type, eUsage usage);

      std::string Semantics() const;
      std::string DeclareVert() const;
      std::string DeclareFrag() const;

   private:
      std::string                m_name;
      std::string                m_type;
      EffectSemantics::eSemantic m_semantic;
      eUsage                     m_usage;
   };

private:
   uint32_t                      m_indent;
   std::ostream                  *m_os;
   const EffectOptions           &m_options;
   const std::vector<PassState>  &m_states;

   std::vector<std::vector<Variable> >    m_variables;      // Attributes and uniforms
   std::vector<std::vector<std::string> > m_varyings;       // Varyings
};
// @endcond

//! These options control the kind of materials that are generated by the ObjFactory.
class ObjMaterialOptions
{
public:
   ObjMaterialOptions() :
      m_numLights(0),
      m_fragmentLighting(false),
      m_map_Kd_format(Image::eRGB888),
      m_passStates(1),
      m_debug(false)
   {
   }

   bool HasLighting() const                                 { return m_numLights != 0;                }

   ObjMaterialOptions &SetNumLights(uint32_t n)             { m_numLights = n; return *this;          }
   uint32_t GetNumLights() const                            { return m_numLights;                     }
      
   ObjMaterialOptions &SetFragmentLighting(bool b)          { m_fragmentLighting = b; return *this;   }
   bool GetFragmentLighting()                const          { return m_fragmentLighting;              }

   ObjMaterialOptions &SetMapKdFormat(Image::eFormat fmt)   { m_map_Kd_format = fmt; return *this;    }
   Image::eFormat GetMapKdFormat() const                    { return m_map_Kd_format;                 }

   const EffectOptions &GetEffectOptions() const            { return m_effectOptions;                 }
   EffectOptions       &GetEffectOptions()                  { return m_effectOptions;                 }

   const PassState &GetPassState() const                    { return m_passStates[0];                 }
   PassState &GetPassState()                                { return m_passStates[0];                 }

   // @cond
   const std::vector<PassState> &GetPassStates() const      { return m_passStates;                    }
   std::vector<PassState> &GetPassStates()                  { return m_passStates;                    }

   bool GetDebug() const   { return m_debug;    }
   void SetDebug(bool dbg) { m_debug = dbg;     }
   // @endcond

private:
   uint32_t                m_numLights;
   bool                    m_fragmentLighting;
   Image::eFormat          m_map_Kd_format;
   EffectOptions           m_effectOptions;
   std::vector<PassState>  m_passStates;
   bool                    m_debug;
};

// @cond

class ObjEffectGenerator : public EffectGenerator
{
public:
   ObjEffectGenerator(const ObjMaterial &material, const ObjMaterialOptions &options);

   // void Generate(ostream &os) -- inherted from base class.

   virtual  void Samplers(uint32_t p);
   virtual  void VertexShader(uint32_t p);
   virtual  void FragmentShader(uint32_t p);

private:
   bool     Texturing() const;
   bool     Lighting() const;
   bool     FragmentLighting() const;
   uint32_t NumLights() const;

private:
   const ObjMaterial          &m_material;
   const ObjMaterialOptions   &m_options;
};

class ObjMaterialFactory
{
public:
   ObjMaterialFactory(const ObjReader &reader, const ObjMaterialOptions &options, const std::string &path);

private:
   void MakeMaterial(const std::string &name, const ObjMaterial &objMaterial, const ObjMaterialOptions &options);

private:
   std::string                   m_path;
   std::vector<MaterialHandle>   m_materials;
};

// @endcond

}

#endif /* __BSG_EFFECT_GENERATOR_H__ */
