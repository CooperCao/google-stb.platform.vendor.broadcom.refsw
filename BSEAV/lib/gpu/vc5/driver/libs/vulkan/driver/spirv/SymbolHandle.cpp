/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/


#include "glsl_primitive_types.auto.h"
#include "libs/util/log/log.h"

#include "SymbolHandle.h"
#include "Nodes.h"
#include "Module.h"
#include "DflowBuilder.h"
#include "Options.h"

#include <sstream>

LOG_DEFAULT_CAT("bvk::comp::SPVSymbol");

namespace bvk {

static const char *GenName(const DflowBuilder &builder, ShaderFlavour flavour,
                           const char *nm, uint32_t location)
{
   // Add the flavour tag "$F", "$V", etc. to the end of the name to make them unique across stages.
   // We don't want the linker matching things up based on names.
   static const char *sFlavourTags[SHADER_FLAVOUR_COUNT] =
   {
      "$V", "$T", "$E", "$G", "$F", "$C"
   };

   // We must have all the names come from the module allocator since the linker will be reading
   // them and the DflowBuilder will be deceased by then.
   spv::string *newName = builder.New<spv::string>(builder.GetArenaAllocator());

   // Don't alter special symbol names (those starting gl_ or $$)
   if (nm == nullptr)
   {
      *newName = std::to_string(location).c_str();
      *newName += sFlavourTags[flavour];
   }
   else
      *newName = nm;

   return newName->c_str();
}

void SymbolHandle::SetLocation(uint32_t location)
{
   auto vi = &m_symbol->u.var_instance;
   vi->layout_loc_specified = true;
   vi->layout_location      = location;
}

SymbolHandle::SymbolHandle(const DflowBuilder &builder)
{
   m_symbol = builder.New<Symbol>();
}

// Create a symbol corresponding to a SPIRV variable
SymbolHandle SymbolHandle::Variable(const DflowBuilder &builder, ShaderFlavour flavour, const char *name,
                                    const NodeVariable *var, SymbolTypeHandle type)
{
   QualifierDecorations  q(var);
   SymbolHandle          symbol(builder);

   const char *nm = GenName(builder, flavour, name, var->GetResultId());

   glsl_symbol_construct_var_instance(symbol, nm, type, &q, nullptr, nullptr);

   int location;
   if (builder.GetModule().GetVarLocation(&location, var))
      symbol.SetLocation(location);

   return symbol;
}

SymbolHandle SymbolHandle::Variable(const DflowBuilder &builder, ShaderFlavour flavour,
                                    const QualifierDecorations &qualifiers, uint32_t location, SymbolTypeHandle type)
{
   SymbolHandle  symbol(builder);
   const char   *nm = GenName(builder, flavour, nullptr, location);

   glsl_symbol_construct_var_instance(symbol, nm, type, &qualifiers, nullptr, nullptr);
   symbol.SetLocation(location);

   return symbol;
}

// Used for internal symbols e.g. "discard", "gl_Position" etc.
SymbolHandle SymbolHandle::Builtin(const DflowBuilder &builder, const char *name,
                                   spv::StorageClass storageClass, SymbolTypeHandle type)
{
   QualifierDecorations q(storageClass);
   SymbolHandle         symbol(builder);

   glsl_symbol_construct_var_instance(symbol, name, type, &q, nullptr, nullptr);

   return symbol;
}

SymbolHandle SymbolHandle::Internal(const DflowBuilder &builder, const char *name, SymbolTypeHandle type)
{
   QualifierDecorations q;
   SymbolHandle         symbol(builder);

   glsl_symbol_construct_var_instance(symbol, name, type, &q, nullptr, nullptr);

   return symbol;
}

SymbolHandle SymbolHandle::SharedBlock(const SymbolListHandle &symbols)
{
   return SymbolHandle(glsl_construct_shared_block(symbols));
}

void SymbolHandle::DebugPrint() const
{
   const char *flavourNames[] = { "TYPE", "INTERFACE_BLOCK", "VAR_INSTANCE",
                                  "PARAM_INSTANCE", "FUNCTION_INSTANCE", "TEMPORARY" };
   const char *typeFlavours[] = { "PRIMITIVE_TYPE", "STRUCT_TYPE", "BLOCK_TYPE",
                                  "ARRAY_TYPE", "FUNCTION_TYPE" };

   if (log_trace_enabled())
   {
      log_trace("Flavour  = %s", flavourNames[GetFlavour()]);
      log_trace("Name     = %s", GetName() ? GetName() : "(null)");

      SymbolTypeHandle  th(GetType());

      if (th)
      {
         log_trace("Type     = %s", typeFlavours[th.GetFlavour()]);
         log_trace("Typename = %s", th.GetName() ? th.GetName() : "(null)");
      }
      else
         log_trace("Type     = (null)");
   }
}

} // namespace bvk
