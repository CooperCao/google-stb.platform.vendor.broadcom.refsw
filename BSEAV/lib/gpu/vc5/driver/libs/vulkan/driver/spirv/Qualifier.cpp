/******************************************************************************************************
 * Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#include "Spirv.h"
#include "Qualifier.h"
#include "Module.h"

namespace bvk
{

FormatQualifier ToFormatQualifier(spv::ImageFormat fmt)
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
   default                              : unreachable();
   }
}

static StorageQualifier ToStorageQualifier(spv::StorageClass storageClass)
{
   switch (storageClass)
   {
   case spv::StorageClass::UniformConstant : return STORAGE_UNIFORM;
   case spv::StorageClass::Input           : return STORAGE_IN;
   case spv::StorageClass::Output          : return STORAGE_OUT;
   // TODO: Don't we need Uniform and StorageBuffer here?

   default : return STORAGE_NONE;
   }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// QualifierDecorations
//
// Is a "Qualifiers" and has a layout qualifier.
///////////////////////////////////////////////////////////////////////////////////////////////////
QualifierDecorations::QualifierDecorations() :
   Qualifiers{}
{
   pq = PREC_HIGHP;
}

QualifierDecorations::QualifierDecorations(spv::StorageClass storageClass) :
   QualifierDecorations()
{
   sq = ToStorageQualifier(storageClass);
}

QualifierDecorations::QualifierDecorations(const NodeVariable *var) :
   QualifierDecorations(var->GetStorageClass())
{
   auto ptrType   = var->GetResultType()->As<const NodeTypePointer *>();
   auto imageType = ptrType->GetType()->TryAs<const NodeTypeImage *>();

   if (imageType != nullptr)
   {
      spv::ImageFormat  fmt = imageType->GetImageFormat();

      if (fmt != spv::ImageFormat::Unknown)
      {
         m_lq.qualified = FORMAT_QUALED;
         m_lq.format    = ToFormatQualifier(fmt);

         lq = &m_lq;
      }
   }

   for (const Decoration *dec : DecorationQuery(var))
      UpdateWith(dec);
}

void QualifierDecorations::UpdateWith(const Decoration *d)
{
   switch (d->GetKind())
   {
   case spv::Decoration::Patch           : aq = AUXILIARY_PATCH;      break;
   case spv::Decoration::Centroid        : aq = AUXILIARY_CENTROID;   break;
   case spv::Decoration::Sample          : aq = AUXILIARY_SAMPLE;     break;
   case spv::Decoration::NoPerspective   : iq = INTERP_NOPERSPECTIVE; break;
   case spv::Decoration::Flat            : iq = INTERP_FLAT;          break;
   case spv::Decoration::Invariant       : invariant = true;          break;
   case spv::Decoration::Restrict        : mq = MEMORY_RESTRICT;      break;
   case spv::Decoration::Volatile        : mq = MEMORY_VOLATILE;      break;
   case spv::Decoration::Coherent        : mq = MEMORY_COHERENT;      break;
   case spv::Decoration::NonWritable     : mq = MEMORY_WRITEONLY;     break;
   case spv::Decoration::NonReadable     : mq = MEMORY_READONLY;      break;
   case spv::Decoration::RelaxedPrecision: pq = PREC_MEDIUMP;         break;

   default: break;
   }
}

}