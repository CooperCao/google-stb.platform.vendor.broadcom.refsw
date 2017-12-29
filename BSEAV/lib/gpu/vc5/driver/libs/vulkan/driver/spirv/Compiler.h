/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#pragma once

#include "Nodes.h"

#include "glsl_dataflow.h"
#include "glsl_basic_block.h"
#include "glsl_symbols.h"
#include "glsl_compiled_shader.h"

#include <list>
#include <vector>

#include "SymbolHandle.h"
#include "BasicBlock.h"
#include "SymbolTypeHandle.h"
#include "DescriptorInfo.h"
#include "CompiledShaderHandle.h"

namespace bvk
{

class Module;
class DflowScalars;
class Module;
class SPVCompiledShader;
class Specialization;

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
   // These are responsible for scoping the memory allocators
   // and other resources used in the back-end compiler
   Compiler(const Module &module, ShaderFlavour flavour, const char *name, const Specialization &specialization,
            bool robustBufferAccess, bool hasDepthStencil, bool multiSampled);
   ~Compiler();

   // Takes a Module that has been created from a SPIRV file and
   // compiles the specified flavour within into a CompiledShader that can
   // later be fed to the linker.
   // Fills in the descriptorTables with the descriptors used in
   // the shader
   CompiledShaderHandle Compile(DescriptorTables *descriptorTables) const;

   ShaderInterfaces *CreateShaderInterfaces(DflowBuilder &builder) const;

private:
   void FillShaderInterface(ShaderInterface *iface, const DflowBuilder &builder,
                            const SymbolListHandle symbols, Map *symbol_ids) const;

   CompiledShaderHandle TryCompile(DescriptorTables *descriptorTables) const;

private:
   const Module         &m_module;
   ShaderFlavour         m_flavour;
   const char           *m_name;
   const Specialization &m_specialization;

   bool m_robustBufferAccess;
   bool m_hasDepthStencil;
   bool m_multiSampled;
};

} // namespace bvk
