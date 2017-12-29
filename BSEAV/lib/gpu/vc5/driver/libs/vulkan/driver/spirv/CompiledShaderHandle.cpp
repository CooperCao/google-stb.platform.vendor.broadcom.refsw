/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#include <new>

#include "CompiledShaderHandle.h"

#include "glsl_compiler.h"

namespace bvk {

CompiledShaderHandle::CompiledShaderHandle(ShaderFlavour flavour, int version) :
   m_shader(glsl_compiled_shader_create(flavour, version))
{
   if (m_shader == nullptr)
      throw std::bad_alloc();
}

void CompiledShaderHandle::Free()
{
   if (m_shader != nullptr)
      glsl_compiled_shader_free(m_shader);
}

}
