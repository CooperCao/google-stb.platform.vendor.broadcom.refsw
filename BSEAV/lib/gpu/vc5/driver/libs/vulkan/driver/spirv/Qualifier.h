/******************************************************************************************************
 * Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#pragma once

#include "glsl_common.h"
#include "Decoration.h"

namespace bvk
{

class NodeVariable;

class QualifierDecorations : public Qualifiers
{
public:
   QualifierDecorations();
   QualifierDecorations(spv::StorageClass storageClass);
   QualifierDecorations(const NodeVariable *var);

   void UpdateWith(const Decoration *d);

private:
   LayoutQualifier m_lq{};
};

extern FormatQualifier ToFormatQualifier(spv::ImageFormat fmt);

}
