/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#include "vulkan.h"

#include <fstream>
#include <iostream>
#include <iomanip>
#include <vector>
#include <list>
#include <map>
#include <stdint.h>
#include <assert.h>
#include <set>
#include <bitset>

#include "Spirv.h"
#include "Module.h"
#include "Allocating.h"
#include "Allocator.h"
#include "ArenaAllocator.h"
#include "LinkResult.h"
#include "Linker.h"
#include "PrimitiveTypes.h"
#include "Options.h"
#include "DescriptorInfo.h"
#include "CompiledShaderHandle.h"
#include "Specialization.h"

#include "glsl_compiler.h"
#include "glsl_compiled_shader.h"
#include "glsl_backend_cfg.h"
#include "glsl_binary_shader.h"
#include "glsl_errors.h"
#include "libs/tools/dqasmc/dqasmc.h"

static bool sQuietMode = false;
static bool sRobustMode = false;

class ModuleData
{
public:
   ModuleData(const std::string &name, bvk::Module *module) :
      m_moduleName(name),
      m_module(module)
   {
   }

   std::string  m_moduleName;
   bvk::Module *m_module;
};

const char *ModelName(spv::ExecutionModel model)
{
   switch (model)
   {
   case spv::ExecutionModel::Vertex:                 return "Vertex";
   case spv::ExecutionModel::TessellationControl:    return "TessControl";
   case spv::ExecutionModel::TessellationEvaluation: return "TessEval";
   case spv::ExecutionModel::Geometry:               return "Geometry";
   case spv::ExecutionModel::Fragment:               return "Fragment";
   case spv::ExecutionModel::GLCompute:              return "Compute";
   case spv::ExecutionModel::Kernel:                 return "Kernel";
   default:                                          return "ERROR";
   }
}

ShaderFlavour ConvertModel(spv::ExecutionModel model)
{
   switch (model)
   {
   case spv::ExecutionModel::Vertex:                 return SHADER_VERTEX;
   case spv::ExecutionModel::TessellationControl:    return SHADER_TESS_CONTROL;
   case spv::ExecutionModel::TessellationEvaluation: return SHADER_TESS_EVALUATION;
   case spv::ExecutionModel::Geometry:               return SHADER_GEOMETRY;
   case spv::ExecutionModel::Fragment:               return SHADER_FRAGMENT;
   case spv::ExecutionModel::GLCompute:              return SHADER_COMPUTE;
   case spv::ExecutionModel::Kernel:
   default:                                          unreachable();
                                                     return SHADER_FLAVOUR_COUNT;
   }
}

struct EntryPointLocator
{
   EntryPointLocator() = default;
   EntryPointLocator(const std::string &moduleName, spv::ExecutionModel model,
      const std::string &entryPoint) :
      m_moduleName(moduleName),
      m_model(model),
      m_entryPoint(entryPoint) {}

   std::string AsString() const
   {
      return m_moduleName + " :: " + ModelName(m_model) + " :: " + m_entryPoint;
   }

   std::string         m_moduleName;
   spv::ExecutionModel m_model;
   std::string         m_entryPoint;
};

static std::set<std::string>          s_moduleNames;
static std::vector<EntryPointLocator> s_entryPoints;
static bool                           s_needEntryPoint = false;

bool ProcessSPIRVFile(const std::string &filename, std::vector<ModuleData> &modules)
{
   // Open the file
   std::ifstream file(filename, std::ifstream::ate | std::ifstream::binary);
   if (!file.is_open())
   {
      std::cerr << "Couldn't open file " << filename.c_str() << std::endl;
      return false;
   }

   std::vector<uint32_t>   words;
   uint32_t size_in_bytes = static_cast<uint32_t>(file.tellg());
   if (size_in_bytes % 4 != 0)
      return false;

   words.resize(size_in_bytes / 4);

   // Read the SPIRV binary data
   file.seekg(0);
   file.read(reinterpret_cast<char *>(words.data()), size_in_bytes);

   bvk::Module *module = ::new bvk::Module(&bvk::g_defaultAllocCallbacks, words.data(), size_in_bytes);

   // Create our SPIRV module from the binary data and add to list
   modules.emplace_back(filename, module);

   return true;
}

bool HasEntryPoint(const std::vector<ModuleData> &modules, const EntryPointLocator &loc)
{
   for (auto &moduleData : modules)
   {
      if (moduleData.m_moduleName == loc.m_moduleName)
      {
         auto &module = moduleData.m_module;

         for (uint32_t ep = 0; ep < module->GetNumEntryPoints(); ep++)
         {
            if (module->GetEntryPointModel(ep) == loc.m_model &&
                module->GetEntryPointName(ep) == loc.m_entryPoint)
               return true;
         }
      }
   }

   return false;
}

bvk::CompiledShaderHandle CompileEntryPoint(
   std::vector<ModuleData> &modules, ShaderFlavour flavour, const EntryPointLocator &loc)
{
   bvk::CompiledShaderHandle result;

   for (auto &moduleData : modules)
   {
      if (moduleData.m_moduleName == loc.m_moduleName)
      {
         std::cout << "Compiling " << loc.AsString().c_str() << std::endl;

         try
         {
            bvk::DescriptorTables                             tables;
            bvk::Specialization                               specialization(nullptr);

            bvk::Compiler compiler(*moduleData.m_module, flavour, loc.m_entryPoint.c_str(),
                                    specialization, sRobustMode, /*hasDepthStencil=*/true,
                                    /*multiSampled=*/false);

            result = compiler.Compile(&tables);
         }
         catch (std::runtime_error &e)
         {
            std::cout << "   Compilation FAILED : " << e.what() << std::endl;
            return bvk::CompiledShaderHandle();
         }
         catch (...)
         {
            std::cout << "   Compilation FAILED" << std::endl;
            return bvk::CompiledShaderHandle();
         }

         std::cout << "   Compiled OK" << std::endl;

         return result;
      }
   }

   return bvk::CompiledShaderHandle();
}

void PrintAllLocators(const std::vector<ModuleData> &modules)
{
   std::cerr << "All entry points:" << std::endl;

   for (auto &moduleData : modules)
   {
      auto &module = moduleData.m_module;

      uint32_t entryPoints = module->GetNumEntryPoints();
      for (uint32_t ep = 0; ep < entryPoints; ep++)
      {
         EntryPointLocator loc(moduleData.m_moduleName, module->GetEntryPointModel(ep),
                               std::string(module->GetEntryPointName(ep).c_str()));

         std::cerr << loc.AsString().c_str() << std::endl;
      }
   }
}

void FindDefaultEntryPoints(const std::vector<ModuleData> &modules)
{
   std::vector<EntryPointLocator>   locs;

   for (auto &moduleData : modules)
   {
      auto &module = moduleData.m_module;

      uint32_t entryPoints = module->GetNumEntryPoints();
      for (uint32_t ep = 0; ep < entryPoints; ep++)
      {
         EntryPointLocator loc(moduleData.m_moduleName, module->GetEntryPointModel(ep),
                               std::string(module->GetEntryPointName(ep).c_str()));
         locs.push_back(loc);
      }
   }

   // Check at most one of each type
   uint32_t modelCounts[static_cast<uint32_t>(spv::ExecutionModel::Kernel) + 1] = {};

   for (auto &ep : locs)
      modelCounts[static_cast<uint32_t>(ep.m_model)]++;

   uint32_t model = 0;
   for (uint32_t cnt : modelCounts)
   {
      if (cnt > 1)
      {
         std::cout << "Multiple " << ModelName(spv::ExecutionModel(model))
                   << " shaders found. Please select just one." << std::endl;
         PrintAllLocators(modules);
         exit(1);
      }
      model++;
   }

   s_entryPoints = locs;
}

bool ParseArgument(const char *argument)
{
   std::string arg(argument);

   if (arg[0] != '-')
   {
      if (s_needEntryPoint)
      {
         auto &curEP = s_entryPoints.back();
         curEP.m_entryPoint = arg;
         s_needEntryPoint = false;
      }
      else
      {
         // Must be an spv filename
         s_entryPoints.emplace_back();
         s_entryPoints.back().m_moduleName = arg;
         s_moduleNames.insert(arg);
      }
   }
   else
   {
      if (arg[1] == 'q')
      {
         sQuietMode = true;
         return true;
      }
      else if (arg[1] == 'r')
      {
         sRobustMode = true;
         return true;
      }

      assert(!s_needEntryPoint);

      auto &curEP = s_entryPoints.back();
      if (arg.length() == 2 && arg[0] == '-')
      {
         switch (arg[1])
         {
         case 'v': curEP.m_model = spv::ExecutionModel::Vertex;                 break;
         case 't': curEP.m_model = spv::ExecutionModel::TessellationControl;    break;
         case 'e': curEP.m_model = spv::ExecutionModel::TessellationEvaluation; break;
         case 'g': curEP.m_model = spv::ExecutionModel::Geometry;               break;
         case 'f': curEP.m_model = spv::ExecutionModel::Fragment;               break;
         case 'c': curEP.m_model = spv::ExecutionModel::GLCompute;              break;
         default: return false;
         }
      }
      else
         return false;

      s_needEntryPoint = true;
   }

   return true;
}

void PrintUsageExit(const char *exe)
{
   std::cerr <<
      "Usage: " << exe << " [options]\n"
      "  <spv_file [entryPointLoc]> <spv_file [entryPointLoc]> ...\n"
      "  Where entryPointLoc is [-v <entryPoint>] [-t <entryPoint>]\n"
      "                         [-e <entryPoint>] [-g <entryPoint>]\n"
      "                         [-f <entryPoint>] [-c <entryPoint>]\n"
      "  Where [options] can contain:\n"
      "    '-q' : (quiet - suppress assembler and ir file output)\n"
      "    '-r' : (with robust buffer access features)\n"
      ;
   exit(1);
}

void OutputIRFiles(const GLSL_PROGRAM_T *p)
{
   if (!sQuietMode)
   {
      static const char *dbsFilenames[SHADER_FLAVOUR_COUNT] = { "v.dbs", "tc.dbs", "te.dbs",
                                                                "g.dbs", "f.dbs",  "c.dbs" };

      for (int i = 0; i < SHADER_FLAVOUR_COUNT; i++)
      {
         if (p->ir->stage[i].ir != NULL)
            glsl_ir_shader_to_file(p->ir->stage[i].ir, dbsFilenames[i]);
      }
   }
}

void OutputAssembly(const BINARY_PROGRAM_T *bin)
{
   if (!sQuietMode && bin)
   {
      for (uint32_t i = SHADER_VERTEX; i <= SHADER_COMPUTE; i++)
      {
         ShaderFlavour flav = static_cast<ShaderFlavour>(i);

         if (flav == SHADER_FRAGMENT)
         {
            std::cout << std::endl << "=== fragment assembly" << std::endl;
            dqasmc_print_basic((uint64_t *)bin->fshader->code,
               bin->fshader->code_size / sizeof(uint64_t));
         }
         else if (bin->vstages[i][0] != NULL)
         {
            for (int j = 0; j < MODE_COUNT; j++)
            {
               std::cout << std::endl << "=== " << glsl_shader_flavour_name(flav)
                         << (j == MODE_BIN ? " (bin)" : "") << " assembly" << std::endl;
               dqasmc_print_basic((uint64_t *)bin->vstages[i][j]->code,
                  bin->vstages[i][j]->code_size / sizeof(uint64_t));
            }
         }
      }
      std::cout << std::endl << std::endl;
   }
}

int main(int argc, char *argv[])
{
   if (argc < 2)
      PrintUsageExit(argv[0]);

   for (int a = 1; a < argc; a++)
      if (!ParseArgument(argv[a]))
         PrintUsageExit(argv[0]);

   // Pretend we are in API scope to prevent false triggers from the allocator checks
   bvk::APIScoper apiScope;

   // Read any environment options first
   bvk::Options::Initialise();

   glsl_prim_init();

   std::vector<ModuleData>   spirvModules;

   // Load all the SPIRV binaries into spirvModules
   for (auto &name : s_moduleNames)
      ProcessSPIRVFile(name, spirvModules);

   // Find default entry points if none provided
   bool haveEntryPoints = false;
   for (auto &entryPoint : s_entryPoints)
   {
      if (!entryPoint.m_entryPoint.empty())
      {
         haveEntryPoints = true;
         break;
      }
   }
   if (!haveEntryPoints)
      FindDefaultEntryPoints(spirvModules);

   bvk::CompiledShaderHandle compiledShaders[SHADER_FLAVOUR_COUNT];

   // Compile the specified entry points
   for (auto &entryPoint : s_entryPoints)
   {
      if (HasEntryPoint(spirvModules, entryPoint))
      {
         ShaderFlavour flavour = ConvertModel(entryPoint.m_model);

         compiledShaders[flavour] = CompileEntryPoint(spirvModules, flavour, entryPoint);
      }
      else
         std::cerr << "ERROR : " << entryPoint.AsString().c_str() << " does not exist" << std::endl;
   }

   // Time to link
   std::cout << "Linking..." << std::endl;

   // Backend shader key (one render target of type F16)
   auto backendKey = [](const GLSL_PROGRAM_T *program) -> GLSL_BACKEND_CFG_T
   {
      return GLSL_BACKEND_CFG_T{ ((GLSL_FB_16 | GLSL_FB_PRESENT) << GLSL_FB_GADGET_S) | GLSL_DISABLE_UBO_FETCH };
   };

   try
   {
      std::bitset<V3D_MAX_ATTR_ARRAYS> attributeRBSwaps;
      bvk::Linker::LinkShaders(compiledShaders, backendKey, nullptr, OutputIRFiles, OutputAssembly, attributeRBSwaps);
   }
   catch (std::runtime_error &e)
   {
      std::cerr << "Linking FAILED" << std::endl << e.what() << std::endl;
      return 1;
   }
   std::cout << "   Linked OK" << std::endl;

   // Clean up
   for (auto &module : spirvModules)
      ::delete module.m_module;

   return 0;
}
