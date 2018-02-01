/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#include "SymbolHandle.h"
#include "SymbolTypeHandle.h"
#include "Nodes.h"

#include "glsl_symbols.h"
#include "glsl_primitive_types.auto.h"
#include "libs/util/log/log.h"
#include "Module.h"
#include "DflowBuilder.h"
#include "Options.h"

#include <sstream>

LOG_DEFAULT_CAT("bvk::comp::SPVSymbol");

namespace bvk {

FormatQualifier ConvertToFormatQualifier(spv::ImageFormat fmt)
{
   switch (fmt)
   {
   // Mandatory formats
   case spv::ImageFormat::Rgba32i       : return FMT_RGBA32I;
   case spv::ImageFormat::Rgba32f       : return FMT_RGBA32F;
   case spv::ImageFormat::Rgba32ui      : return FMT_RGBA32UI;
   case spv::ImageFormat::Rgba16f       : return FMT_RGBA16F;
   case spv::ImageFormat::R32f          : return FMT_R32F;
   case spv::ImageFormat::Rgba8         : return FMT_RGBA8;
   case spv::ImageFormat::Rgba8Snorm    : return FMT_RGBA8_SNORM;
   case spv::ImageFormat::Rgba16i       : return FMT_RGBA16I;
   case spv::ImageFormat::Rgba8i        : return FMT_RGBA8I;
   case spv::ImageFormat::R32i          : return FMT_R32I;
   case spv::ImageFormat::Rgba16ui      : return FMT_RGBA16UI;
   case spv::ImageFormat::Rgba8ui       : return FMT_RGBA8UI;
   case spv::ImageFormat::R32ui         : return FMT_R32UI;
   // Optional formats
   case spv::ImageFormat::Rg32f         : return FMT_RG32F;
   case spv::ImageFormat::Rg16f         : return FMT_RG16F;
   case spv::ImageFormat::R11fG11fB10f  : return FMT_R11G11B10F;
   case spv::ImageFormat::R16f          : return FMT_R16F;
   case spv::ImageFormat::Rgba16        : return FMT_RGBA16;
   case spv::ImageFormat::Rgb10A2       : return FMT_RGB10A2;
   case spv::ImageFormat::Rg16          : return FMT_RG16;
   case spv::ImageFormat::Rg8           : return FMT_RG8;
   case spv::ImageFormat::R16           : return FMT_R16;
   case spv::ImageFormat::R8            : return FMT_R8;
   case spv::ImageFormat::Rgba16Snorm   : return FMT_RGBA16_SNORM;
   case spv::ImageFormat::Rg16Snorm     : return FMT_RG16_SNORM;
   case spv::ImageFormat::Rg8Snorm      : return FMT_RG8_SNORM;
   case spv::ImageFormat::R16Snorm      : return FMT_R16_SNORM;
   case spv::ImageFormat::R8Snorm       : return FMT_R8_SNORM;
   case spv::ImageFormat::Rg32i         : return FMT_RG32I;
   case spv::ImageFormat::Rg16i         : return FMT_RG16I;
   case spv::ImageFormat::Rg8i          : return FMT_RG8I;
   case spv::ImageFormat::R16i          : return FMT_R16I;
   case spv::ImageFormat::R8i           : return FMT_R8I;
   case spv::ImageFormat::Rgb10a2ui     : return FMT_RGB10A2UI;
   case spv::ImageFormat::Rg32ui        : return FMT_RG32UI;
   case spv::ImageFormat::Rg16ui        : return FMT_RG16UI;
   case spv::ImageFormat::Rg8ui         : return FMT_RG8UI;
   case spv::ImageFormat::R16ui         : return FMT_R16UI;
   case spv::ImageFormat::R8ui          : return FMT_R8UI;
   default                          : unreachable();
   }
}

class SPVQualifiers
{
public:
   SPVQualifiers()
   {
      m_qual.invariant = false;
      m_qual.sq        = STORAGE_NONE;
      m_qual.iq        = INTERP_SMOOTH;
      m_qual.aq        = AUXILIARY_NONE;
      m_qual.lq        = nullptr;
      m_qual.pq        = PREC_HIGHP;
      m_qual.mq        = MEMORY_NONE;
   }

   SPVQualifiers(spv::StorageClass storageClass) :
      SPVQualifiers()
   {
      switch (storageClass)
      {
      case spv::StorageClass::UniformConstant : m_qual.sq = STORAGE_UNIFORM; break;
      case spv::StorageClass::Input           : m_qual.sq = STORAGE_IN;      break;
      case spv::StorageClass::Output          : m_qual.sq = STORAGE_OUT;     break;
      // TODO: Don't we need Uniform and StorageBuffer here?

      default : break;
      }
   }

   void SetImageFormat(spv::ImageFormat fmt)
   {
      if (fmt != spv::ImageFormat::Unknown)
      {
         m_layoutQual.qualified = FORMAT_QUALED;
         m_layoutQual.format = ConvertToFormatQualifier(fmt);
         m_qual.lq = &m_layoutQual;
      }
   }

   Qualifiers &GetQualifiers() { return m_qual; }
   const Qualifiers &GetQualifiers() const { return m_qual; }

private:
   Qualifiers        m_qual;
   LayoutQualifier   m_layoutQual{};
};

class QualifierGather : public DecorationVisitor
{
public:
   QualifierGather(const Module &module, SPVQualifiers *qual) :
      DecorationVisitor(module),
      m_qual(*qual)
   {
   }

   void Visit(const Decoration &d) override;

private:
   SPVQualifiers   &m_qual;
};

void QualifierGather::Visit(const Decoration &decoration)
{
   switch (decoration.GetKind())
   {
   case spv::Decoration::Patch         : m_qual.GetQualifiers().aq = AUXILIARY_PATCH;      break;
   case spv::Decoration::Centroid      : m_qual.GetQualifiers().aq = AUXILIARY_CENTROID;   break;
   case spv::Decoration::Sample        : m_qual.GetQualifiers().aq = AUXILIARY_SAMPLE;     break;
   case spv::Decoration::NoPerspective : m_qual.GetQualifiers().iq = INTERP_NOPERSPECTIVE; break;
   case spv::Decoration::Flat          : m_qual.GetQualifiers().iq = INTERP_FLAT;          break;
   case spv::Decoration::Invariant     : m_qual.GetQualifiers().invariant = true;          break;
   case spv::Decoration::Restrict      : m_qual.GetQualifiers().mq = MEMORY_RESTRICT;      break;
   case spv::Decoration::Volatile      : m_qual.GetQualifiers().mq = MEMORY_VOLATILE;      break;
   case spv::Decoration::Coherent      : m_qual.GetQualifiers().mq = MEMORY_COHERENT;      break;
   case spv::Decoration::NonWritable   : m_qual.GetQualifiers().mq = MEMORY_WRITEONLY;     break;
   case spv::Decoration::NonReadable   : m_qual.GetQualifiers().mq = MEMORY_READONLY;      break;

   default: break;
   }
}

static const char *sFlavourTags[SHADER_FLAVOUR_COUNT] =
{
   "$V",
   "$T",
   "$E",
   "$G",
   "$F",
   "$C"
};

static const char *GenName(const Module &module, ShaderFlavour flavour,
                           const char *nm, const NodeVariable *var/* = nullptr*/)
{
   // Add the flavour tag "$F", "$V", etc. to the end of the name to make them unique across stages.
   // We don't want the linker matching things up based on names.

   // We must have all the names come from the module allocator since the linker will be reading
   // them and the DflowBuilder will be deceased by then.
   spv::string *newName = module.New<spv::string>(module.GetArenaAllocator());

   // Don't alter special symbol names (those starting gl_ or $$)
   if (strncmp(nm, "gl_", 3) && strncmp(nm, "$$", 2))
   {
      if (Options::fullSymbolNames || var == nullptr)
         *newName = nm;
      else
         // Use the node id as the symbol name
         *newName = std::to_string(var->GetResultId()).c_str();

      *newName += sFlavourTags[flavour];
   }
   else
      *newName = nm;

   return newName->c_str();
}

SymbolHandle::SymbolHandle(const Module &module)
{
   m_symbol = module.New<Symbol>();
}

// Create a symbol corresponding to a SPIRV variable
SymbolHandle SymbolHandle::Variable(const Module &module, ShaderFlavour flavour, const char *name,
                                    const NodeVariable *var, SymbolTypeHandle type)
{
   SPVQualifiers q(var->GetStorageClass());

   auto ptrType   = var->GetResultType()->As<const NodeTypePointer *>();
   auto imageType = ptrType->GetType()->TryAs<const NodeTypeImage *>();
   if (imageType != nullptr)
      q.SetImageFormat(imageType->GetImageFormat());

   QualifierGather(module, &q).Foreach(var);

   SymbolHandle symbol(module);

   const char *nm = GenName(module, flavour, name, var);

   glsl_symbol_construct_var_instance(symbol, nm, type, &q.GetQualifiers(), nullptr, nullptr);

   auto &vi = symbol.m_symbol->u.var_instance;
   vi.layout_loc_specified = module.GetVarLocation(&vi.layout_location, var);

   return symbol;
}

// Used for internal symbols e.g. "discard", "gl_Position" etc.
SymbolHandle SymbolHandle::Builtin(const Module &module, const char *name,
                                   spv::StorageClass storageClass, SymbolTypeHandle type)
{
   SPVQualifiers    q(storageClass);
   SymbolHandle     symbol(module);

   glsl_symbol_construct_var_instance(symbol, name, type, &q.GetQualifiers(), nullptr, nullptr);

   return symbol;
}

SymbolHandle SymbolHandle::Internal(const Module &module, const char *name, SymbolTypeHandle type)
{
   SPVQualifiers q;
   SymbolHandle  symbol(module);

   glsl_symbol_construct_var_instance(symbol, name, type, &q.GetQualifiers(), nullptr, nullptr);

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
