/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#pragma once

#include "glsl_dataflow.h"
#include "glsl_basic_block.h"
#include "glsl_symbols.h"
#include "glsl_compiled_shader.h"
#include "glsl_binary_program.h"
#include "glsl_backend_cfg.h"

#include "SymbolHandle.h"
#include "BasicBlock.h"
#include "SymbolTypeHandle.h"
#include "CompiledShaderHandle.h"

#include <functional>
#include <bitset>

typedef struct CompiledShader_s  CompiledShader;

namespace bvk {

class Linker
{
public:
   using BuildFunc          = std::function<void(const IR_PROGRAM_T &ir, const BINARY_PROGRAM_T &prog)>;
   using BackendKeyFunc     = std::function<GLSL_BACKEND_CFG_T(const GLSL_PROGRAM_T *prog)>;
   using OutputIRFunc       = std::function<void(const GLSL_PROGRAM_T *prog)>;
   using OutputAssemblyFunc = std::function<void(const BINARY_PROGRAM_T *prog)>;

   static void LinkShaders(CompiledShaderHandle shaders[SHADER_FLAVOUR_COUNT],
                           BackendKeyFunc backendFn,
                           BuildFunc buildFn, OutputIRFunc irFunc, OutputAssemblyFunc assFunc,
                           const std::bitset<V3D_MAX_ATTR_ARRAYS> &attribRBSwaps);
};

} // namespace bvk
