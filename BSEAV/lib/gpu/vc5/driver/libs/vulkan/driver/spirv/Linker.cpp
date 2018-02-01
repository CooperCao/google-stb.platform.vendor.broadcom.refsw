/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#include "Linker.h"
#include "CompiledShaderHandle.h"

namespace bvk {

static void ReplaceMaps(int *savedUniformMaps[SHADER_FLAVOUR_COUNT], int *savedBufferMaps[SHADER_FLAVOUR_COUNT],
                        GLSL_PROGRAM_T *prog)
{
   // Yuck - we need to replace the link_map->uniforms table with an identity map
   // of 32 entries. This prevents us having to change the backend compiler code.
   static int identityMap[32] = {
       0,  1,  2,  3,  4,  5,  6,  7,  8,  9,
      10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
      20, 21, 22, 23, 24, 25, 26, 27, 28, 29,
      30, 31 };

   for (uint32_t s = 0; s < SHADER_FLAVOUR_COUNT; s++)
   {
      if (prog->ir->stage[s].link_map)
      {
         savedUniformMaps[s] = prog->ir->stage[s].link_map->uniforms;
         prog->ir->stage[s].link_map->num_uniforms = 32;
         prog->ir->stage[s].link_map->uniforms     = identityMap;

         savedBufferMaps[s] = prog->ir->stage[s].link_map->buffers;
         prog->ir->stage[s].link_map->num_buffers = 32;
         prog->ir->stage[s].link_map->buffers     = identityMap;
      }
   }
}

static void RestoreMaps(int *savedUniformMaps[SHADER_FLAVOUR_COUNT], int *savedBufferMaps[SHADER_FLAVOUR_COUNT],
                        GLSL_PROGRAM_T *prog)
{
   // Restore the link maps we diddled with
   for (uint32_t s = 0; s < SHADER_FLAVOUR_COUNT; s++)
      if (prog->ir->stage[s].link_map)
      {
         prog->ir->stage[s].link_map->uniforms = savedUniformMaps[s];
         prog->ir->stage[s].link_map->buffers  = savedBufferMaps[s];
      }
}

void Linker::LinkShaders(CompiledShaderHandle shaders[SHADER_FLAVOUR_COUNT],
                         BackendKeyFunc backendFn, OutputFunc outputFn,
                         const std::bitset<V3D_MAX_ATTR_ARRAYS> &attribRBSwaps)
{
   GLSL_PROGRAM_T *prog;
   if (shaders[SHADER_COMPUTE])
   {
      prog = glsl_link_compute_program(shaders[SHADER_COMPUTE]);
   }
   else
   {
      GLSL_PROGRAM_SOURCE_T notSrc = {}; // Only needed for transform-feedback

      // The cast is safe if the handle type just holds the pointer
      // The alternative is to copy all the pointers into a temporary array
      prog = glsl_link_program(reinterpret_cast<CompiledShader **>(shaders), &notSrc, /*separable=*/false, /*validate=*/false);
   }

   if (prog == nullptr)
      throw std::runtime_error(std::string("Link Error : ") + glsl_compile_error_get());

   if (shaders[SHADER_VERTEX])
   {
      // RB swap all attribute locations
      int swap[4] = { 2, 1, 0, 3 };
      for (int i=0; i<prog->ir->stage[SHADER_VERTEX].link_map->num_ins; i++)
      {
         int loc  = prog->ir->stage[SHADER_VERTEX].link_map->ins[i] / 4;
         int comp = prog->ir->stage[SHADER_VERTEX].link_map->ins[i] & 3;
         if (attribRBSwaps.test(loc))
            comp = swap[comp];
         prog->ir->stage[SHADER_VERTEX].link_map->ins[i] = 4 * loc + comp;
      }
   }

   // Replace uniform map with an identity table of 32 entries
   int *savedUniformMaps[SHADER_FLAVOUR_COUNT] = {};
   int *savedBufferMaps[SHADER_FLAVOUR_COUNT]  = {};
   ReplaceMaps(savedUniformMaps, savedBufferMaps, prog);

   // Run the backend code generator
   GLSL_BACKEND_CFG_T cfg = backendFn(prog);
   BINARY_PROGRAM_T *binaryProg = glsl_binary_program_from_dataflow(prog->ir, &cfg);

   // Call the callback to process the finished program
   if (outputFn)
      outputFn(prog->ir, binaryProg);

   glsl_binary_program_free(binaryProg);

   // Restore the uniform maps we replaced earlier
   RestoreMaps(savedUniformMaps, savedBufferMaps, prog);

   glsl_program_free(prog);

   if (binaryProg == nullptr)
      throw std::runtime_error(std::string("CodeGen Error"));
}

} // namespace bvk
