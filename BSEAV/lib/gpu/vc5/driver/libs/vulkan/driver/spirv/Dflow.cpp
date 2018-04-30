/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#include "Dflow.h"
#include "DflowScalars.h"
#include "BasicBlock.h"
#include "libs/util/demand.h"
#include "libs/khrn/glsl/dflib.h"

namespace bvk {

Dflow Dflow::Default(DataflowType dfType)
{
   switch (dfType)
   {
   case DF_BOOL:
   case DF_INT:
   case DF_UINT:
   case DF_FLOAT:
      return Dflow::Value(dfType, 0u);

   case DF_SAMPLER:
      return Dflow::Sampler(-1);

   case DF_F_SAMP_IMG:
   case DF_I_SAMP_IMG:
   case DF_U_SAMP_IMG:
   case DF_F_STOR_IMG:
   case DF_I_STOR_IMG:
   case DF_U_STOR_IMG:
      return Dflow::ImageUniform(dfType, /*relaxed=*/true, -1);

   default:
      assert(0);
      return Dflow();
   }
}

Dflow Dflow::Vec4(const DflowScalars &coord)
{
   Dataflow *u = coord.Size() > 0 ? coord[0].m_dflow : nullptr;
   Dataflow *v = coord.Size() > 1 ? coord[1].m_dflow : nullptr;
   Dataflow *s = coord.Size() > 2 ? coord[2].m_dflow : nullptr;
   Dataflow *t = coord.Size() > 3 ? coord[3].m_dflow : nullptr;

   return Dflow(glsl_dataflow_construct_vec4(u, v, s, t));
}

Dflow Dflow::Atomic(DataflowFlavour flavour, DataflowType type,
                    const Dflow &addr, const Dflow &data,
                    BasicBlockHandle block)
{
   Dataflow *s  = glsl_dataflow_construct_atomic(flavour, type, addr, data, /*cond=*/nullptr,
                                                 block->GetMemoryAccessChain());
   block->SetMemoryAccessChain(s);

   return Dflow(s);
}

Dflow Dflow::PackImageData(FormatQualifier f, const DflowScalars &data)
{
   return Dflow(dflib_pack_format(f, data.Data()));
}

Dflow Dflow::CreateImageWriteAddress(const Dflow &image, const DflowScalars &coord)
{
   return Dflow(glsl_dataflow_construct_texture_addr(image, coord[0], coord[1], coord[2], coord[3]));
}

} // namespace bvk
