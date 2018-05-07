/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#include <new>
#include <algorithm>

#ifdef WIN32
#pragma warning(disable : 4800)  // From autogen'd code
#endif

#include "glsl_fastmem.h"
#include "glsl_safemem.h"
#include "glsl_intern.h"
#include "glsl_compiler.h"
#include "glsl_binary_program.h"
#include "glsl_binary_shader.h"
#include "glsl_symbols.h"
#include "glsl_fastmem.h"
#include "glsl_primitive_types.auto.h"

#include "libs/util/log/log.h"

#include "NonCopyable.h"
#include "Compiler.h"
#include "DflowBuilder.h"

namespace bvk
{

Compiler::Compiler(const Module &module, ShaderFlavour flavour, const char *name, const Specialization &specialization,
                   const Controls &controls) :
   m_module(module),
   m_flavour(flavour),
   m_name(name),
   m_specialization(specialization),
   m_controls(controls)
{
   glsl_fastmem_init();
   glsl_dataflow_begin_construction();
   glsl_init_intern(1024);
   glsl_dataflow_reset_count();
}

Compiler::~Compiler()
{
   glsl_dataflow_end_construction();
   glsl_fastmem_term();
#ifndef NDEBUG
   glsl_safemem_verify();
#endif
}

class ShUsageHandle
{
private:
   sh_usage m_usage;

   void FillShaderInterface(if_usage *iface, const DflowBuilder &builder, const SymbolListHandle &symbols) const;
   void Cleanup();

public:
   ShUsageHandle(const DflowBuilder &builder);
   ~ShUsageHandle() { Cleanup(); }

   operator const sh_usage *() const { return &m_usage; }
};

ShUsageHandle::ShUsageHandle(const DflowBuilder &builder)
{
   FillShaderInterface(&m_usage.in,      builder, builder.GetInputSymbols());
   FillShaderInterface(&m_usage.out,     builder, builder.GetOutputSymbols());
   FillShaderInterface(&m_usage.uniform, builder, builder.GetUniformSymbols());

   m_usage.buffer.n = 0;

   if (m_usage.in.v == nullptr || m_usage.out.v == nullptr || m_usage.uniform.v == nullptr)
   {
      Cleanup();
      throw std::bad_alloc();
   }
}

void ShUsageHandle::Cleanup()
{
   delete [] m_usage.in.v;
   delete [] m_usage.out.v;
   delete [] m_usage.uniform.v;
}

void ShUsageHandle::FillShaderInterface(if_usage *iface, const DflowBuilder &builder, const SymbolListHandle &symbols) const
{
   iface->n = symbols.size();
   iface->v = new (std::nothrow) symbol_usage [iface->n];
   if (iface->v == nullptr)
      return;

   unsigned j = 0;
   for (const auto &sym : symbols)
   {
      iface->v[j].symbol = sym;
      iface->v[j].used   = builder.HasStaticUse(sym);
      j++;
   }
}

CompiledShaderHandle Compiler::TryCompile(DescriptorTables *descriptorTables, uint32_t sharedMemPerCore) const
{
   // Build the dataflow for the entry point in the SPIRV module
   DflowBuilder builder(descriptorTables, m_module, m_specialization,
                        m_controls.IsRobust(), m_controls.IsMultisampled());

   builder.Build(m_name, m_flavour, sharedMemPerCore);

   ShUsageHandle symbs(builder);

   IFaceData ifaceData = builder.GetExecutionModes();

   if (m_flavour == SHADER_COMPUTE)
      ifaceData.cs.shared_block_size = builder.GetWorkgroup().GetBlockSize();

   CompiledShaderHandle ret(glsl_compile_common(m_flavour, 310, builder.GetEntryBlock()->GetBlock(), &ifaceData, symbs, builder.GetSymbolIdMap(), m_controls.DoUnroll(), /*activity_supported=*/false));

   if (m_flavour == SHADER_FRAGMENT)
      ret->u.fs.early_tests = ret->u.fs.early_tests || !m_controls.HasDepthStencil();

   return ret;
}

CompiledShaderHandle Compiler::Compile(DescriptorTables *descriptorTables, uint32_t sharedMemPerCore) const
{
   try
   {
      return TryCompile(descriptorTables, sharedMemPerCore);
   }
   catch (const std::bad_alloc &)
   {
      throw;
   }
   catch (...)
   {
      throw std::runtime_error(std::string("Compilation of ") + glsl_shader_flavour_name(m_flavour) +
                               " shader using entry point '" + m_name + "' failed");
   }
}

} // namespace bvk
