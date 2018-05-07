/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#pragma once

#include "glsl_symbols.h"
#include "glsl_compiled_shader.h"

#include "SymbolHandle.h"
#include "CompiledShaderHandle.h"

namespace bvk
{

class Module;
class Specialization;
class DflowBuilder;
class DescriptorTables;

///////////////////////////////////////////////////////////////////
// Compiler
//
// Handles the actual compilation of the the Dataflow into a
// CompiledShader which can be fed to the linker.
//
// This class helps to isolate the interface to the existing glsl
// compiler from the rest of the code.
///////////////////////////////////////////////////////////////////
class Compiler
{
public:
   class Controls
   {
   public:
      Controls() = default;
      Controls(const Controls &rhs) = default;

      Controls &SetRobustBufferAccess(bool b) { m_robustBufferAccess = b; return *this; }
      Controls &SetDepthStencil(bool b)       { m_hasDepthStencil    = b; return *this; }
      Controls &SetMultisampled(bool b)       { m_multisampled       = b; return *this; }
      Controls &SetUnroll(bool b)             { m_unroll             = b; return *this; }

      bool IsRobust()        const { return m_robustBufferAccess; }
      bool HasDepthStencil() const { return m_hasDepthStencil;    }
      bool IsMultisampled()  const { return m_multisampled;       }
      bool DoUnroll()        const { return m_unroll;             }

   private:
      bool m_robustBufferAccess{};
      bool m_hasDepthStencil{};
      bool m_multisampled{};
      bool m_unroll{true};
   };

   // These are responsible for scoping the memory allocators
   // and other resources used in the back-end compiler
   Compiler(const Module &module, ShaderFlavour flavour, const char *name,
            const Specialization &specialization, const Controls &controls);
   ~Compiler();

   // Uses the Module that has been created from a SPIRV file and
   // compiles the specified flavour within into a CompiledShader that can
   // later be fed to the linker.
   // Fills in the descriptorTables with the descriptors used in
   // the shader
   CompiledShaderHandle Compile(DescriptorTables *descriptorTables, uint32_t sharedMemPerCore) const;

private:
   void FillShaderInterface(if_usage *iface, const DflowBuilder &builder, const SymbolListHandle &symbols) const;

   CompiledShaderHandle TryCompile(DescriptorTables *descriptorTables, uint32_t sharedMemPerCore) const;

private:
   const Module         &m_module;
   ShaderFlavour         m_flavour;
   const char           *m_name;
   const Specialization &m_specialization;

   Controls              m_controls;
};

} // namespace bvk
