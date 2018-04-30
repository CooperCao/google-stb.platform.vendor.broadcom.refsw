/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#pragma once

#include "NonCopyable.h"

#include "glsl_ir_program.h"

typedef struct CompiledShader_s   CompiledShader;

namespace bvk {

class CompiledShaderHandle : public NonCopyable
{
public:
   CompiledShaderHandle() :
      m_shader(nullptr)
   {}

   CompiledShaderHandle(ShaderFlavour flavour, int version);

   ~CompiledShaderHandle()
   {
      Free();
   }

   // Could be moved to change ownership
   CompiledShaderHandle(CompiledShaderHandle &&rhs) :
      m_shader(rhs.m_shader)
   {
      rhs.m_shader = nullptr;
   }

   CompiledShaderHandle &operator=(CompiledShaderHandle &&rhs)
   {
      if (&rhs != this)
      {
         Free();
         m_shader = rhs.m_shader;
         rhs.m_shader = nullptr;
      }

      return *this;
   }

   operator CompiledShader *()  { return m_shader;            }
   explicit operator bool()     { return m_shader != nullptr; }
   CompiledShader *operator->() { return m_shader;            }

   explicit CompiledShaderHandle(CompiledShader *shader) :
      m_shader(shader)
   {}

private:
   void Free();

private:
   CompiledShader *m_shader;
};

// This ensures that we can safely cast arrays of CompiledShaderHandle
// to arrays of CompiledShader *
static_assert(sizeof(CompiledShaderHandle) == sizeof(CompiledShader *),
              "CompiledShaderHandle is not the size of a pointer");

}
