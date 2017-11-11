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
#include "glsl_basic_block_elim_dead.h"
#include "glsl_intern.h"
#include "glsl_basic_block_flatten.h"
#include "glsl_primitive_type_index.auto.h"
#include "glsl_compiler.h"
#include "glsl_ssa_convert.h"
#include "glsl_dominators.h"
#include "glsl_dataflow_cse.h"
#include "glsl_dataflow_visitor.h"
#include "glsl_binary_program.h"
#include "glsl_binary_shader.h"
#include "glsl_shader_interfaces.h"
#include "glsl_symbols.h"
#include "glsl_fastmem.h"

#include "libs/util/log/log.h"

#include "NonCopyable.h"
#include "Compiler.h"
#include "DflowBuilder.h"
#include "PrimitiveTypes.h"

LOG_DEFAULT_CAT("bvk::comp::Compiler");

namespace bvk
{

Compiler::Compiler(const Module &module, ShaderFlavour flavour, const char *name, const Specialization &specialization,
                   bool robustBufferAccess, bool hasDepthStencil, bool multiSampled) :
   m_module(module),
   m_flavour(flavour),
   m_name(name),
   m_specialization(specialization),
   m_robustBufferAccess(robustBufferAccess),
   m_hasDepthStencil(hasDepthStencil),
   m_multiSampled(multiSampled)
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

ShaderInterfaces *Compiler::CreateShaderInterfaces(DflowBuilder &builder) const
{
   ShaderInterfaces *interfaces = glsl_shader_interfaces_new();

   for (auto sym : builder.GetOutputSymbols())
      glsl_shader_interfaces_update(interfaces, sym);
   for (auto sym : builder.GetInputSymbols())
      glsl_shader_interfaces_update(interfaces, sym);
   for (auto sym : builder.GetUniformSymbols())
      glsl_shader_interfaces_update(interfaces, sym);

   return interfaces;
}

void Compiler::FillShaderInterface(ShaderInterface *iface, const DflowBuilder &builder,
                                   const SymbolListHandle symbols, Map *symbol_ids) const
{
   if (symbols.begin() != symbols.end())
   {
      iface->var = static_cast<InterfaceVar*>(malloc(symbols.size() * sizeof(InterfaceVar)));
      if (iface->var == nullptr)
         throw std::bad_alloc();
   }
   else
      iface->var = nullptr;

   iface->n_vars = symbols.size();

   int i = 0;
   for (const auto &sym : symbols)
   {
      InterfaceVar *v = &iface->var[i];

      v->symbol      = sym;
      v->active      = false;
      v->static_use  = builder.HasStaticUse(sym);
#if V3D_VER_AT_LEAST(4,0,2,0)
      v->flags       = nullptr;
#endif

      v->ids = static_cast<int*>(malloc(sym.GetNumScalars() * sizeof(int)));
      if (v->ids == nullptr)
         throw std::bad_alloc();

      for (unsigned j = 0; j < sym.GetNumScalars(); j++)
         v->ids[j] = -1;

      int *ids = static_cast<int*>(glsl_map_get(symbol_ids, sym));
      if (ids)
      {
         for (unsigned j = 0; j < sym.GetNumScalars(); j++)
            v->ids[j] = ids[j];

         v->active = true;
      }

      i++;
   }
}

static void DebugMap(const char *label, Map *map)
{
   if (log_trace_enabled())
   {
      log_trace("%s", label);
      GLSL_MAP_FOREACH(e, map)
      {
         Symbol *sym = (Symbol*)e->k;
         log_trace("Symbol = %s, Type = %s, scalars = %u", sym->name, sym->type->name, sym->type->scalar_count);
      }
   }
}

class SSAShaderHandle : public NonCopyable
{
public:
   SSAShaderHandle()
   {}

   ~SSAShaderHandle()
   {
      glsl_ssa_shader_term(&m_irsh);
   }

   operator SSAShader *()  { return &m_irsh; }
   SSAShader *operator->() { return &m_irsh; }

private:
   SSAShader m_irsh = {};
};

class MapHandle : public NonCopyable
{
public:
   MapHandle() :
      m_map(glsl_map_new())
   {
      if (m_map == nullptr)
         throw std::bad_alloc();
   }

   ~MapHandle()
   {
      glsl_map_delete(m_map);
   }

   operator Map *() { return m_map; }

private:
   Map *m_map;
};

CompiledShaderHandle Compiler::TryCompile(DescriptorTables *descriptorTables) const
{
   // Build the dataflow for the entry point in the SPIRV module
   DflowBuilder builder(descriptorTables, m_module, m_specialization,
                        m_robustBufferAccess, m_multiSampled);

   builder.Build(m_name, m_flavour);

   Map *symbol_ids = builder.GetSymbolIdMap();

   BasicBlock *entry_block = builder.GetEntryBlock()->GetBlock();
   glsl_basic_block_elim_dead(entry_block);

   DebugMap("== Entry block scalars (pre-flatten)", entry_block->scalar_values);
   DebugMap("== Entry block loads", entry_block->loads);

   bool ssa_flattening = glsl_ssa_flattening_supported(entry_block);
   if (!ssa_flattening)
      entry_block = glsl_basic_block_flatten(entry_block);

   DebugMap("== Entry block scalars (post flatten)", entry_block->scalar_values);
   DebugMap("== Entry block loads", entry_block->loads);

   // Do the conversion to SSA form
   SSAShaderHandle ir_sh;
   glsl_ssa_convert(ir_sh, entry_block, builder.GetOutputSymbols(), symbol_ids);
   glsl_ssa_shader_optimise(ir_sh, /* mem_read-only =*/ false, ssa_flattening);

   glsl_ssa_shader_hoist_loads(ir_sh);

   // Destroy the list of BasicBlocks now that ir_sh has been created
   glsl_basic_block_delete_reachable(entry_block);

   // Copy our interfaces into glsl interfaces
   ShaderInterfaces *interfaces = CreateShaderInterfaces(builder);

   CompiledShaderHandle ret(m_flavour, 310); // TODO use the right version

   {
      MapHandle symbol_map;

      if (!glsl_copy_compiled_shader(ret, interfaces, symbol_map))
         throw std::bad_alloc();
   }

   FillShaderInterface(&ret->uniform,  builder, builder.GetUniformSymbols(), symbol_ids);
   FillShaderInterface(&ret->in,       builder, builder.GetInputSymbols(),   symbol_ids);
   FillShaderInterface(&ret->out,      builder, builder.GetOutputSymbols(),  symbol_ids);

   // This seems to be unnecessary, and it causes problems for some tests see SWVC5-817
   // TODO: revisit why and fix properly
   // glsl_mark_interface_actives(&ret->in, &ret->uniform, &ret->buffer, ir_sh.blocks, ir_sh.n_blocks);

   if (!glsl_copy_shader_ir(ret, ir_sh))
      throw std::bad_alloc();

   if (m_flavour == SHADER_FRAGMENT)
   {
      bool visible_effects = false;
      for (int i = 0; i < ret->num_cfg_blocks; i++)
      {
         CFGBlock *b = &ret->blocks[i];
         for (int j = 0; j < b->num_dataflow; j++)
            if (glsl_dataflow_affects_memory(b->dataflow[j].flavour))
               visible_effects = true;
      }

      ret->early_fragment_tests = !m_hasDepthStencil ||
                              (builder.GetExecutionModes().HasEarlyFragTests() || !visible_effects);

      ret->abq = ADV_BLEND_NONE;
   }

   if (m_flavour == SHADER_TESS_CONTROL)
   {
      ret->tess_vertices = 0;                   // TODO
   }

   if (m_flavour == SHADER_TESS_EVALUATION)
   {
      ret->tess_mode = V3D_CL_TESS_TYPE_ISOLINES;         // TODO
      ret->tess_spacing = V3D_CL_TESS_EDGE_SPACING_EQUAL; // TODO
      ret->tess_cw = false;                               // TODO
      ret->tess_point_mode = false;                       // TODO
   }

   if (m_flavour == SHADER_GEOMETRY)
   {
      ret->gs_in = GS_IN_POINTS;                  // TODO
      ret->gs_out = V3D_CL_GEOM_PRIM_TYPE_POINTS; // TODO
      ret->gs_n_invocations = 1;                  // TODO
      ret->gs_max_vertices = 0;                   // TODO
   }

   if (m_flavour == SHADER_COMPUTE)
   {
      for (int i = 0; i < 3; ++i)
         ret->cs_wg_size[i] = builder.GetExecutionModes().GetWorkgroupSize()[i];

      ret->cs_shared_block_size = builder.GetWorkgroup().GetBlockSize();
   }

   return ret;
}

CompiledShaderHandle Compiler::Compile(DescriptorTables *descriptorTables) const
{
   try
   {
      return TryCompile(descriptorTables);
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
